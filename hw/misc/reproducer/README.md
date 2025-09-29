# TCG + PhysMem Bug Reproducer

This directory contains a complete reproducer setup for demonstrating TCG + physmem bugs in QEMU through memory region overlap scenarios.

## Architecture

The reproducer consists of five main components:

### Devices

- **`trigger-device.c`**: 4-byte MMIO device at `0x40000000` that triggers remote device resizing
- **`mapper-device.c`**: Contains a 16KiB alias at `0x40001000` and manages secondary address space  
- **`remote-device.c`**: Resizable device (4KiB→16KiB) that creates overlap with alias
- **`reproducer-soc.c`**: SoC that integrates all devices with proper property linking
- **`reproducer-board.c`**: Machine type that instantiates the SoC

### Memory Layout

```
Primary Address Space:
  0x40000000-0x40000003: reproducer-trigger-mmio (4 bytes)
  0x40001000-0x40004fff: alias @reproducer-secondary-space 0x2000-0x5fff (16KB)

Secondary Address Space:  
  0x0000-0x0fff: reproducer-remote-mmio (4KB initially)
  0x0000-0x3fff: reproducer-remote-mmio (16KB after trigger)
  
Overlap Created:
  Secondary 0x2000-0x3fff overlaps with alias 0x2000-0x3fff (8KB overlap)
```

## Files

### Core Implementation

- `trigger-device.c/h` - Trigger device implementation
- `mapper-device.c/h` - Mapper device with alias and secondary space
- `remote-device.c/h` - Resizable remote device  
- `reproducer-soc.c/h` - SoC integration
- `reproducer-board.c` - Machine type registration

### Build System

- `meson.build` - Build configuration
- `Kconfig` - Configuration options
- `trace-events` - Trace event definitions
- `trace.h` - Trace header

### Test Programs

- `simple_reproducer_test.s` - Simple ARM assembly test program
- `simple_reproducer_test.bin` - Compiled ROM image (44 bytes)
- `reproducer_test.s` - More complex ARM test program  
- `reproducer_test.ld` - Linker script for ROM layout

### Scripts

- `build.sh` - Complete build script for QEMU with reproducer
- `test_reproducer.sh` - Test script for running reproducer
- `debug_reproducer.sh` - Debug script with tracing

### Documentation

- `context.md` - Detailed implementation context
- `README.md` - This file

## Building

```bash
# Build QEMU with reproducer support
./build.sh

# Or manually from QEMU root:
cd ../../..
./configure --target-list=arm-softmmu --enable-debug --enable-trace-backends=simple
make -j$(nproc) qemu-system-arm
```

## Testing

```bash
# Run reproducer board
./build/qemu-system-arm -M reproducer -nographic -monitor stdio

# In QEMU monitor:
(qemu) info mtree                    # Show initial memory layout
(qemu) o/w 0x40000000 0x12345678    # Trigger resize
(qemu) info mtree                    # Show overlap condition
(qemu) quit
```

## Bug Reproduction

The reproducer creates the exact conditions needed to trigger TCG + physmem bugs:

1. **Initial State**: Remote device occupies 4KiB (0x0-0xfff), alias maps 0x2000-0x5fff
2. **Trigger Write**: Write to 0x40000000 resizes remote device to 16KiB (0x0-0x3fff)  
3. **Overlap Created**: Remote device and alias both claim 0x2000-0x3fff (8KiB overlap)
4. **Bug Condition**: Memory access through alias hits overlapped region

This setup exposes memory management bugs in TCG's handling of overlapping memory regions.

## Trace Events

Enable tracing to see device operations:
```bash
./build/qemu-system-arm -M reproducer -trace 'trigger_*,mapper_*,remote_*'
```

Available trace events:
- `trigger_device_write` - Trigger device write operations
- `mapper_device_read/write` - Mapper alias operations  
- `remote_device_read/write` - Remote device operations
- `remote_device_resize` - Device resize events
