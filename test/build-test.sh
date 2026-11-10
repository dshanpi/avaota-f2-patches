#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "Usage: sh $0 /absolute/path/to/tina-v861-v1.1" >&2
    exit 1
fi

sdk_dir=$(CDPATH= cd -- "$1" && pwd)
test_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
compiler=${CC:-"$sdk_dir/prebuilt/rootfsbuilt/riscv/Xuantie-900-gcc-linux-6.6.36-musl32-x86_64-V3.3.0/bin/riscv32-unknown-linux-musl-gcc"}
if [ ! -x "$compiler" ]; then
    echo "Cross compiler not found: $compiler" >&2
    echo "Set CC to the SDK's RISC-V32 musl cross compiler." >&2
    exit 1
fi

"$compiler" -march=rv32imafdc -mabi=ilp32d -Os -s -static \
    "$test_dir/gc2093_raw_test.c" -o "$test_dir/gc2093_raw_test"
echo "Built: $test_dir/gc2093_raw_test"
