# TCG + PhysMem Bug Reproducer

This directory contains a complete reproducer setup for demonstrating TCG + physmem bugs in QEMU through asynchronous memory region modification that causes TCG dispatch map staleness.

## Bug Description

Based on the analysis in [anatomy-of-a-bug](http://127.0.0.1:1111/posts/master/anatomy-of-a-bug/), this reproducer targets a specific bug where:

1. A timer callback modifies a secondary address space asynchronously (simulating LibAFL fuzzer hooks)
2. This modification happens **without** triggering TCG commit handlers on the primary address space
3. When the CPU later accesses memory through an alias region, the TCG dispatch map is stale
4. This causes an assertion failure in `iotlb_to_section()`: `assert(section_index < d->map.sections_nb)`

## Architecture

The reproducer consists of five main components that create the exact conditions for the bug:

### Devices

- **`trigger-device.c`**: 4KiB MMIO device at `0x40000000` that starts async timer when written to
- **`mapper-device.c`**: Contains a 16KiB alias at `0x40001000` and manages secondary address space
- **`remote-device.c`**: Resizable device (4KiB→32KiB) in secondary address space 
- **`reproducer-soc.c`**: SoC that integrates all devices with proper property linking
- **`reproducer-board.c`**: ARM Cortex-A9 machine type with ROM/RAM

### Memory Layout

```
Primary Address Space (CPU):
  0x00000000-0x000FFFFF: ROM (1MB) - Test program loading area
  0x10000000-0x13FFFFFF: RAM (64MB) - System memory  
  0x40000000-0x40000FFF: reproducer-trigger-mmio (4KiB)
  0x40001000-0x40004FFF: alias @reproducer-secondary-space (16KiB)

Secondary Address Space (mapper-device owned):
  0x0000-0x0FFF: reproducer-remote-mmio (4KiB initially)
  0x0000-0x7FFF: reproducer-remote-mmio (32KiB after timer fires)
  
Bug Trigger Sequence:
1. Write to 0x40000000 → starts timer
2. Timer callback directly calls memory_region_set_size() on remote device
3. Remote device expands from 4KiB to 32KiB in secondary address space
4. CPU access through alias hits stale TCG dispatch map → assertion failure
```

## Files

### Core Implementation

- `trigger-device.c` + `include/hw/misc/reproducer/trigger-device.h`
- `mapper-device.c` + `include/hw/misc/reproducer/mapper-device.h`  
- `remote-device.c` + `include/hw/misc/reproducer/remote-device.h`
- `reproducer-soc.c` + `include/hw/misc/reproducer/reproducer-soc.h`
- `reproducer-board.c` - Machine type registration

### Build System

- `meson.build` - Build configuration integrated with QEMU build system
- `Kconfig` - Configuration options (`CONFIG_REPRODUCER`)
- `trace-events` - Modern trace event definitions with `reproducer_` prefix
- `trace.h` - Trace include wrapper

### Test Programs (`testcase/` directory)

- **C Version (primary)**:
  - `reproducer_test.c` - Main C test program with proper bug triggering sequence
  - `startup.s` - ARM startup code for C runtime
  - `reproducer_test.ld` - Linker script for ROM layout
  - `Makefile` - Cross-compilation build system for ARM test binaries
  
- **Assembly Version (legacy)**:
  - `simple_reproducer_test.s` - Simple ARM assembly test
  - `reproducer_test.s` - Complex assembly test (legacy)

### Scripts

- `build.sh` - Complete build script for QEMU with reproducer
- `test_reproducer.sh` - Test script for running reproducer
- `debug_reproducer.sh` - Debug script with tracing

### Documentation

- `context.md` - Detailed implementation context
- `README.md` - This file

## Building

### QEMU Build

```bash
# Build QEMU with reproducer support
./build.sh

# Or manually from QEMU root:
cd ../../..
./configure --target-list=arm-softmmu --enable-debug --enable-trace-backends=simple
make -j$(nproc) qemu-system-arm
```

### Test Program Build

```bash
# Build ARM test programs (requires arm-none-eabi-gcc)
cd testcase/
make all

# Or build specific targets:
make reproducer_test.bin      # C version 
make simple_reproducer_test.bin  # Assembly version
```

## Testing

### Basic Functionality Test

```bash
# Run reproducer board
./build/qemu-system-arm -M reproducer -nographic -monitor stdio

# In QEMU monitor:
(qemu) info mtree                    # Show initial memory layout
(qemu) o/w 0x40000000 0x12345678    # Trigger timer → async resize
(qemu) info mtree                    # Should show expanded remote device
(qemu) quit
```

### Bug Reproduction Test

```bash
# Run with C test program that triggers the bug
./build/qemu-system-arm -M reproducer -nographic \
    -bios hw/misc/reproducer/testcase/reproducer_test.bin

# Expected behavior: assertion failure in iotlb_to_section()
# Actual observation: [document current behavior]
```

### Debug with Tracing

```bash
# Full trace analysis
./build/qemu-system-arm -M reproducer -nographic \
    --trace "reproducer_*" \
    -bios hw/misc/reproducer/testcase/reproducer_test.bin
```

## Current Status

### ✅ Implemented Features

- **Device Architecture**: All 5 devices properly implemented and integrated
- **Timer-Based Async Modification**: Timer callback directly modifies secondary address space
- **C Test Program**: Complete test program with startup code and proper trigger sequence
- **Build System**: Fully integrated with QEMU build system and ARM cross-compilation
- **Trace Events**: Comprehensive tracing for debugging device interactions

### 🔍 Investigation Needed

- **Bug Reproduction**: Current testing does not reliably trigger the assertion failure
- **Timer Timing**: May need fine-tuning of timer delay to create the exact race condition
- **Memory Access Pattern**: The specific access pattern that causes TCG dispatch staleness

### 🚧 Next Steps

1. **Enhanced Testing**: More complex memory access patterns after timer triggers
2. **Timer Variations**: Different timer delays and multiple timer approach
3. **Address Space Analysis**: Better understanding of when TCG commit handlers fire
4. **Alternative Triggers**: Investigation of other scenarios that cause dispatch staleness

## Bug Reproduction Strategy

The key insight from the blog post is that the bug occurs when:

```c
// This happens in timer callback (simulating fuzzer callback):
memory_region_transaction_begin();
memory_region_set_size(&remote_device->mmio, expanded_size);  // Direct modification
memory_region_transaction_commit();
// No TCG commit handler triggered because we're not in primary AS context!

// Later, CPU access through alias:
volatile uint32_t value = read32(ALIAS_REGION_BASE);  // Should trigger assertion
```

The current implementation follows this pattern but may need refinement in timing or access patterns.

## Trace Events

Available trace events (use `--trace "reproducer_*"`):

- `reproducer_trigger_device_write` - Trigger device write operations
- `reproducer_trigger_device_timer_resize` - Timer callback start
- `reproducer_trigger_device_timer_complete` - Timer callback completion
- `reproducer_remote_device_read/write` - Remote device operations
- `reproducer_remote_device_resize_start/resized` - Device resize events
- `reproducer_mapper_device_read/write` - Mapper alias operations
