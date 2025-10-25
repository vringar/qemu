#!/bin/bash

# Debug script for reproducer ARM CPU boot investigation
# Usage: ./debug_reproducer.sh [--new]
#   --new: Use the new C version (reproducer_test.bin) instead of assembly version
# This will run QEMU under GDB to investigate why ARM CPU doesn't execute ROM

# Parse command line arguments
USE_NEW_VERSION=false
if [[ "$1" == "--new" ]]; then
    USE_NEW_VERSION=true
    echo "Using new C version: reproducer_test.bin"
else
    echo "Using assembly version: simple_reproducer_test.bin"
fi

# Set BIOS file based on version
if [[ "$USE_NEW_VERSION" == "true" ]]; then
    BIOS_FILE="reproducer_test.bin"
else
    BIOS_FILE="simple_reproducer_test.bin"
fi

# Get the directory of this script and determine paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QEMU_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
TESTCASE_DIR="$SCRIPT_DIR"
BIOS_PATH="$TESTCASE_DIR/$BIOS_FILE"

echo "Script dir: $SCRIPT_DIR"
echo "QEMU root: $QEMU_ROOT"
echo "Using BIOS: $BIOS_PATH"

# Change to QEMU root directory
cd "$QEMU_ROOT"

echo "Reproducer ARM CPU Boot Debug"
echo "============================="
echo "Testing version: $BIOS_FILE"
echo "Investigating why ARM CPU doesn't execute loaded ROM content"
echo ""

# Option 1: Quick ROM content check
echo "1. Quick ROM content verification:"
cat > rom_check.gdb << 'EOF'
set confirm off
target remote :1234
# Check what's loaded at ROM base
echo \n=== ROM CONTENT CHECK ===\n
x/10i 0x0
echo \n=== CPU STATE ===\n
info registers
echo \n=== MEMORY LAYOUT ===\n
info mem
echo \n=== PC LOCATION ===\n
print $pc
x/5i $pc
quit
EOF

echo "   Starting QEMU with GDB server..."
./build/qemu-system-arm -M reproducer -nographic -bios "$BIOS_PATH" -s -S &
QEMU_PID=$!
sleep 2

echo "   Connecting with GDB..."
gdb-multiarch --batch --command=rom_check.gdb ./build/qemu-system-arm 2>/dev/null
kill $QEMU_PID 2>/dev/null
wait $QEMU_PID 2>/dev/null

echo ""
echo "2. Interactive debugging session:"
echo "   To debug interactively, run:"
echo "   Terminal 1: ./build/qemu-system-arm -M reproducer -nographic -bios \"$BIOS_PATH\" -s -S"
echo "   Terminal 2: gdb-multiarch"
echo "              (gdb) target remote localhost:1234"
echo "              (gdb) x/5i 0x0          # Check ROM content"
echo "              (gdb) info registers    # Check CPU state"
echo "              (gdb) stepi             # Step through instructions"
echo "              (gdb) continue          # Continue execution"

echo ""
echo "3. Trace debugging:"
echo "   To debug with tracing:"
echo "   ./build/qemu-system-arm -M reproducer -nographic --trace \"reproducer_*\" -bios \"$BIOS_PATH\""

echo ""
echo "4. Monitor debugging:"
echo "   Use QEMU monitor commands:"
echo "   (qemu) info mtree       # Check memory layout"
echo "   (qemu) x/5i 0x0         # Disassemble ROM"
echo "   (qemu) info registers   # Check CPU state"
echo "   (qemu) x/x 0x40000000   # Read trigger device"

# Create helper script for device testing
cat > device_debug.gdb << 'EOF'
set confirm off
target remote :1234
# Set breakpoints on device access
break trigger_device_write
break remote_device_resize
break mapper_device_write
echo \n=== BREAKPOINTS SET FOR DEVICE ACCESS ===\n
info breakpoints
echo \n=== CONTINUING - WILL BREAK ON DEVICE ACCESS ===\n
continue
EOF

echo ""
echo "5. Device access debugging:"
echo "   To debug device access, use device_debug.gdb:"
echo "   Terminal 1: ./build/qemu-system-arm -M reproducer -nographic -bios \"$BIOS_PATH\" -s"
echo "   Terminal 2: gdb-multiarch --command=device_debug.gdb ./build/qemu-system-arm"

echo ""
echo "Debug setup completed. Helper files created:"
echo "- rom_check.gdb: Quick ROM verification"
echo "- device_debug.gdb: Device access debugging"

# Clean up rom_check.gdb but keep device_debug.gdb for manual use
rm -f rom_check.gdb

echo ""
echo "Current Investigation Status:"
echo "- ✅ BIOS ($BIOS_FILE) loads successfully to ROM at 0x0"
echo "- ✅ Memory layout correct (ROM, RAM, devices)"
echo "- ✅ ARM Cortex-A9 CPU configured"
if [[ "$USE_NEW_VERSION" == "true" ]]; then
    echo "- ✅ C version with proper startup code"
else
    echo "- ✅ Assembly version tested"
fi
echo "- 🔍 CPU doesn't automatically execute ROM content"
echo "- 🔍 Need to investigate ARM reset vectors and boot sequence"
