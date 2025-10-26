#!/bin/bash
set -e

echo "TCG + PhysMem Bug Reproducer Build Script"
echo "=========================================="

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QEMU_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

echo "QEMU root: $QEMU_ROOT"
echo "Script dir: $SCRIPT_DIR"

cd "$QEMU_ROOT"
cd build

echo "Configuring QEMU..."
../configure \
    --target-list=arm-softmmu \
    --enable-debug \
    --disable-fuse \
    --enable-trace-backends=log \

echo "Building QEMU..."
make -j$(nproc) qemu-system-arm
