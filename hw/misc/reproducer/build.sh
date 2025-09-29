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

# Clean up old build
echo "Cleaning old build directory..."
rm -rf build
mkdir -p build
cd build

echo "Configuring QEMU..."
../configure \
    --target-list=arm-softmmu \
    --enable-debug \
    --disable-fuse \
    --enable-trace-backends=log \

echo "Building QEMU..."
make -j$(nproc) qemu-system-arm

echo ""
echo "Build completed successfully!"
echo ""
echo "To test the reproducer devices, run:"
echo "  ./qemu-system-arm -M reproducer -nographic -monitor stdio"
echo ""
echo "To test with tracing (working syntax with explicit log backend):"
echo "  ./qemu-system-arm -M reproducer -nographic --trace \"reproducer_*\" -bios hw/misc/reproducer/simple_reproducer_test.bin"
echo ""
echo "Alternative tracing with file output:"
echo "  ./qemu-system-arm -M reproducer -nographic --trace \"reproducer_*\" -D trace.log -bios hw/misc/reproducer/simple_reproducer_test.bin"
echo ""
echo "In the QEMU monitor, you can:"
echo "  (qemu) info mtree"
echo "  (qemu) x/x 0x40000000    # Read trigger device"
echo "  (qemu) o/w 0x40000000 1  # Write to trigger device (triggers resize)"
echo "  (qemu) info mtree        # See the changed memory layout"
