#!/usr/bin/env python3
"""Build the runtime add-ons; copy the unmodified SDK sample into a deployable tar."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

HERE = Path(__file__).resolve().parent
ISP_REL = Path('platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp')
HEADER_REL = ISP_REL / 'isp_cfg/SENSOR_H/ov13850/ov13850_mipi_isp610_20261021_195907.h'
APP_REL = Path('platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/bin/sample_smartIPC_demo')
TESTED_APP_SHA256 = 'f450f0cb19e613a5b667fa9260fd96613960de6d733a6726b77021cddf245cb0'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def run(args):
    subprocess.run([str(x) for x in args], check=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True, help='Tina V861 v1.1 SDK root')
    parser.add_argument('--out', type=Path, default=HERE / 'build', help='build directory (default: ./build)')
    parser.add_argument('--cross-cc', type=Path, help='riscv32 musl GCC, override SDK compiler detection')
    parser.add_argument('--host-cc', default=os.environ.get('HOST_CC', 'gcc'))
    args = parser.parse_args()
    if sys.byteorder != 'little':
        parser.error('The ISP exporter requires a little-endian host (tested: Ubuntu x86_64)')
    sdk, out = args.sdk.expanduser().resolve(), args.out.expanduser().resolve()
    if out == HERE or out == sdk or sdk in out.parents:
        parser.error('--out must be outside the SDK and must not be this source directory')
    header, app, isp = sdk / HEADER_REL, sdk / APP_REL, sdk / ISP_REL
    for f in [header, app, isp / 'include/isp_ini_parse.h']:
        if not f.is_file():
            parser.error(f'Missing {f}; first apply the repository source files and build the SDK/MPP sample')
    elf = app.read_bytes()[:20]
    if len(elf) < 20 or elf[:6] != b'\x7fELF\x01\x01' or int.from_bytes(elf[18:20], 'little') != 243:
        parser.error('SDK sample must be a 32-bit little-endian RISC-V ELF')
    if args.cross_cc:
        cc = args.cross_cc.expanduser().resolve()
    else:
        candidates = sorted((sdk / 'prebuilt/rootfsbuilt/riscv').glob('*/bin/riscv32-unknown-linux-musl-gcc'))
        if len(candidates) != 1:
            parser.error(f'Found {len(candidates)} SDK cross compilers; select one using --cross-cc')
        cc = candidates[0]
    if not cc.is_file():
        parser.error(f'Missing compiler: {cc}')

    work, runtime = out / '.work', out / 'ov13850-display'
    allowed = {'BUILD-INFO.json', 'SHA256SUMS', 'run.sh', 'stop.sh',
               'app/sample_smartIPC_demo', 'lib/librtsp_tcp_writeall.so',
               'profiles/1080p30.conf', 'profiles/1440p30.conf', 'profiles/4k25.conf',
               'iq25/isp_param_config.bin', 'iq30/isp_param_config.bin'}
    if runtime.exists():
        unknown = [str(f.relative_to(runtime)) for f in runtime.rglob('*')
                   if f.is_file() and f.relative_to(runtime).as_posix() not in allowed]
        if unknown:
            parser.error(f'Unexpected files in runtime output; choose a new --out directory: {unknown}')
    for d in [work, runtime, runtime/'app', runtime/'lib', runtime/'profiles', runtime/'iq25', runtime/'iq30']:
        d.mkdir(parents=True, exist_ok=True)
    # Preserve the original header. Normalize its stale relative include only in a build copy.
    raw = header.read_bytes()
    old = b'#include "../../include/isp_ini_parse.h"'
    if raw.count(old) != 1:
        parser.error('Unexpected ISP header include; this exporter targets the repository ISP610 header')
    (work/'vendor-reference.h').write_bytes(raw.replace(old, b'#include "include/isp_ini_parse.h"'))
    run([args.host_cc, '-DISP_VERSION=610', '-I'+str(work), '-I'+str(isp),
         '-I'+str(isp/'include'), '-I'+str(isp/'isp_tuning'), HERE/'src/export-isp.c', '-o', work/'export-isp'])
    for fps in [25, 30]:
        target = runtime/f'iq{fps}/isp_param_config.bin'
        run([work/'export-isp', target, fps])
        if target.stat().st_size != 164058:
            raise RuntimeError('Unexpected ISP export length')
    run([cc, '-shared', '-fPIC', '-Os', '-s', '-march=rv32imafdc', '-mabi=ilp32d',
         HERE/'src/rtsp-tcp-writeall.c', '-o', runtime/'lib/librtsp_tcp_writeall.so', '-ldl', '-pthread'])
    for mode in ['1080p30', '1440p30', '4k25']:
        shutil.copyfile(HERE/'profiles'/f'{mode}.conf', runtime/'profiles'/f'{mode}.conf')
    for name in ['run.sh', 'stop.sh']:
        shutil.copyfile(HERE/name, runtime/name)
        (runtime/name).chmod(0o755)
    shutil.copyfile(app, runtime/'app/sample_smartIPC_demo')
    (runtime/'app/sample_smartIPC_demo').chmod(0o755)
    info = {'sdk_sample_relative_path': str(APP_REL), 'sample_sha256': sha(app),
            'matches_tested_sample': sha(app) == TESTED_APP_SHA256,
            'isp_header_sha256': sha(header), 'cross_compiler': str(cc),
            'cross_compiler_version': subprocess.check_output([str(cc),'--version'],text=True).splitlines()[0],
            'host_byte_order': sys.byteorder, 'runtime_iq_abi_size': 163984}
    (runtime/'BUILD-INFO.json').write_text(json.dumps(info, indent=2)+'\n')
    files = sorted(f for f in runtime.rglob('*') if f.is_file() and f.name != 'SHA256SUMS')
    (runtime/'SHA256SUMS').write_text(''.join(sha(f)+'  '+f.relative_to(runtime).as_posix()+'\n' for f in files))
    archive = out/'ov13850-runtime.tar'
    with tarfile.open(archive, 'w', format=tarfile.USTAR_FORMAT) as tar:
        for f in files + [runtime/'SHA256SUMS']:
            tar.add(f, arcname=f.relative_to(runtime).as_posix(), recursive=False)
    (out/'SHA256SUMS').write_text(sha(archive)+'  '+archive.name+'\n')
    if not info['matches_tested_sample']:
        print('NOTE: SDK sample hash differs from the tested sample; validate API, symbols, IQ loading and all modes again.', file=sys.stderr)
    print(f'Runtime: {runtime}\nTransfer only: {archive}')

if __name__ == '__main__':
    main()
