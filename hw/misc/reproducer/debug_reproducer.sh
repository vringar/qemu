#!/bin/bash

# Debug script for reproducer assertion failure
# This will run QEMU under GDB and catch the assertion to show the callstack

cd /run/media/stefan/02e400c2-1bdd-4d46-a0dd-044d6b4f3af4/Projekte/qemu

# Create GDB command file
cat > debug_commands.gdb << 'EOF'
# Set up to catch the assertion
break qdev_realize
condition 1 dev != 0 && DEVICE_GET_CLASS(dev)->bus_type != 0

# Also catch the actual assertion failure
break __assert_fail

# Run the program
run -machine reproducer -nographic -serial mon:stdio

# When we hit a breakpoint, show backtrace and relevant info
commands 1
  echo \n=== ASSERTION ABOUT TO TRIGGER ===\n
  print dev
  print *dev
  if dev != 0
    print DEVICE_GET_CLASS(dev)
    if DEVICE_GET_CLASS(dev) != 0
      print DEVICE_GET_CLASS(dev)->bus_type
    end
  end
  echo \n=== BACKTRACE ===\n
  bt
  echo \n=== CONTINUING ===\n
  continue
end

commands 2
  echo \n=== ASSERTION FAILED ===\n
  echo \nAssertion details:\n
  info registers
  echo \n=== FULL BACKTRACE ===\n
  bt
  echo \n=== FRAME DETAILS ===\n
  bt full
  echo \n=== STOPPING HERE ===\n
  quit
end

# Continue execution
continue
EOF

echo "Starting GDB debugging session..."
echo "This will catch the assertion and show the callstack"
echo "Press Ctrl+C to interrupt if needed"

gdb --batch --command=debug_commands.gdb ./build/qemu-system-arm

# Clean up
rm -f debug_commands.gdb

echo ""
echo "Debug session completed."