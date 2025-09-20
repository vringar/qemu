#!/bin/bash

# Test the reproducer device functionality
cd /run/media/stefan/02e400c2-1bdd-4d46-a0dd-044d6b4f3af4/Projekte/qemu

# Start QEMU with the reproducer board and execute some commands
echo "Testing Reproducer Device Functionality:"
echo "1. Starting QEMU with reproducer board..."

# Create a test script that writes to the trigger device to resize the remote device
cat > test_commands.txt << 'EOF'
# Connect to the monitor
info mtree
# Write to trigger device at 0x40000000 to trigger resize
x/1w 0x40000000
# Try to write to the trigger (this should trigger resize)
# Note: QEMU monitor doesn't easily support memory writes, so we'll just check the memory map
info mtree
quit
EOF

echo "2. Running test commands..."
timeout 30s ./build/qemu-system-arm -machine reproducer -nographic -serial mon:stdio < test_commands.txt

echo "3. Test completed!"

# Clean up
rm -f test_commands.txt
