#!/usr/bin/env python3
"""Generate a text script for a Linux serial console; no ADB or TF required."""
from pathlib import Path
import hashlib
import sys

test_dir = Path(__file__).resolve().parent
output = Path(sys.argv[1]) if len(sys.argv) > 1 else test_dir / 'uart-upload.txt'
binary = test_dir / 'gc2093_raw_test'
if not binary.is_file():
    raise SystemExit('Build gc2093_raw_test with build-test.sh first.')
files = {name: (test_dir / name).read_bytes() for name in
         ['gc2093_raw_test', 'run-camera-test.sh', 'raw-1080p30.conf']}
files['raw-1080p30.conf'] = files['raw-1080p30.conf'].replace(
    b'OUTPUT_DIR=/mnt/extsd/gc2093-raw-test', b'OUTPUT_DIR=/tmp/gc2093-raw-test')
lines = ['mkdir -p /tmp/gc2093-test']
for name, data in files.items():
    dest = '/tmp/gc2093-test/' + name
    lines.append(': > ' + dest)
    for offset in range(0, len(data), 32):
        escaped = ''.join('\\%03o' % value for value in data[offset:offset + 32])
        lines.append("printf '%b' '" + escaped + "' >> " + dest)
lines += ['chmod +x /tmp/gc2093-test/gc2093_raw_test',
          'md5sum /tmp/gc2093-test/gc2093_raw_test',
          'echo GC2093_UPLOAD_DONE']
assert max(map(len, lines)) < 256
output.write_bytes(('\n'.join(lines) + '\n').encode('ascii'))
print('Send as text to the Linux serial shell:', output)
print('Use LF line endings and a line delay of at least 20 ms.')
print('Expected executable MD5:', hashlib.md5(binary.read_bytes()).hexdigest())
print('Then run: cd /tmp/gc2093-test; sh run-camera-test.sh')
