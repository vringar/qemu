# TCG + PhysMem Bug Reproducer - Project Context

## Project Overview

### Goal

Create a minimal reproducer device for bugs in the TCG (Tiny Code Generator) + physmem (physical memory) subsystems using QEMU system emulation with ARM Cortex-A9.

### Architecture Requirements

- **ReproducerBoard**: ARM machine with Cortex-A9 CPU, ROM, and RAM
- **ReproducerSoC**: Main SoC containing test devices (no CPU - moved to board level)
- **TriggerDevice**: 4-byte MMIO device that triggers RemoteDevice resize
- **MapperDevice**: Owns secondary address space (2MiB) and creates 16KiB alias into CPU space
- **RemoteDevice**: MMIO device with logging that can resize from 4KiB to 16KiB

### Memory Layout Design

```
CPU Address Space:
- 0x00000000-0x000FFFFF: ROM (reproducer.rom) - BIOS/firmware loading
- 0x10000000-0x13FFFFFF: RAM (reproducer.ram) - 64MB system memory
- 0x40000000: TriggerDevice (4 bytes)
- 0x40001000: MapperDevice alias (16KiB)

Secondary Address Space (owned by MapperDevice):
- 0x0000: RemoteDevice (4KiB initially, 16KiB after resize)
- 0x2000: Alias offset in secondary space

Bug Reproduction Mechanism:
- Initial: RemoteDevice [0x0000-0x0FFF], Alias [0x2000-0x5FFF]
- After resize: RemoteDevice [0x0000-0x3FFF], creates 8KiB overlap [0x2000-0x3FFF]
- This dynamic resize while CPU has cached translations should expose TCG/physmem bugs
```

## Implementation Status

### ✅ Completed Components

#### Directory Structure

- `/hw/misc/reproducer/` - All implementation and header files (consolidated)
- Fully integrated into QEMU build system with modern trace support

#### Files Created

```
hw/misc/reproducer/
├── reproducer-soc.c              # SoC with devices (no CPU)
├── reproducer-board.c            # Board with ARM Cortex-A9 CPU + memory
├── trigger-device.c              # Trigger device implementation
├── mapper-device.c               # Mapper device with alias
├── remote-device.c               # Resizable remote device
├── trace-events                  # Modern trace event definitions
├── trace.h                       # Trace include wrapper
├── meson.build                   # Build configuration
├── Kconfig                       # Configuration options
├── build.sh                      # Build script with modern trace syntax
├── simple_reproducer_test.s      # ARM assembly test program
├── simple_reproducer_test.bin    # Compiled ARM test binary
├── compile_test.sh               # Test compilation script
├── context.md                    # This documentation
├── test_reproducer.sh            # Test script
├── debug_reproducer.sh           # Debug script
└── README.md                     # Usage documentation
```

#### Build System Integration

- ✅ Added `hw/misc/reproducer` to `trace_events_subdirs` in main `meson.build`
- ✅ Updated `hw/misc/meson.build` to include reproducer subdirectory
- ✅ Updated `hw/misc/Kconfig` to source reproducer Kconfig
- ✅ Created `CONFIG_REPRODUCER` config option (default y, depends on ARM)
- ✅ Modern trace system integration with `reproducer_*` prefix
- ✅ All trace events available via `--trace help | grep reproducer`

### ✅ Resolved Issues

#### Header Include Problems (FIXED)
```c
// ✅ RESOLVED: Proper includes now working
#include "system/memory.h"      // Modern path
#include "hw/core/cpu.h"        // For cpu_set_pc
#include "hw/loader.h"          // For load_image_targphys
```

#### Function Signature Issues (FIXED)
```c
// ✅ RESOLVED: ObjectClass callback signatures
+ CORRECT: void (*)(ObjectClass *klass, const void *data)
```

#### Modern Trace System (FIXED)
```c
// ✅ RESOLVED: Modern trace system working
// All reproducer_* events available
// Usage: --trace "reproducer_*"
```

#### CPU and Memory Configuration (FIXED)
```c
// ✅ RESOLVED: ARM Cortex-A9 CPU properly configured
// ✅ RESOLVED: ROM and RAM regions working
// ✅ RESOLVED: Memory region conflicts resolved
```

### 🚧 Current Investigation

#### ARM CPU Boot Sequence
- **Issue**: CPU doesn't automatically execute loaded ROM content
- **Status**: BIOS loads successfully, memory layout correct
- **Next**: Investigation into ARM reset vectors and boot sequence

## Device Implementation Details

### RemoteDevice
- **Type**: `TYPE_REMOTE_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **MMIO**: Initially 4KiB, expandable to 16KiB
- **Function**: `remote_device_resize()` - Public function to trigger resize
- **Status**: ✅ Compiles and builds successfully

### TriggerDevice
- **Type**: `TYPE_TRIGGER_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **MMIO**: 4 bytes at 0x40000000
- **Function**: Any write triggers `remote_device_resize()`
- **Status**: ✅ Compiles and builds successfully

### MapperDevice
- **Type**: `TYPE_MAPPER_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **Components**: 2MiB secondary space, 16KiB alias region at 0x40001000
- **Status**: ✅ Compiles and builds successfully

### ReproducerSoC
- **Type**: `TYPE_REPRODUCER_SOC`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **Components**: TriggerDevice, MapperDevice (CPU moved to board level)
- **Status**: ✅ Compiles and builds successfully

### ReproducerBoard
- **Machine Type**: `"reproducer"`
- **CPU**: ARM Cortex-A9 with proper reset configuration
- **Memory**: ROM (0x0) + RAM (0x10000000) + devices
- **Function**: Instantiates CPU, memory, and ReproducerSoC
- **Status**: ✅ Compiles and builds successfully

## Build Instructions

### Configuration

```bash
# Use the provided build script
cd hw/misc/reproducer/
./build.sh

# Or manually:
cd qemu/
rm -rf build && mkdir build && cd build
../configure \
    --target-list=arm-softmmu \
    --enable-debug \
    --enable-trace-backends=simple \
    --disable-fuse
make -j$(nproc)
```

### Usage

```bash
# Run with reproducer machine (basic)
./qemu-system-arm -M reproducer -nographic -monitor stdio

# Run with modern tracing and BIOS
./qemu-system-arm -M reproducer -nographic --trace "reproducer_*" -bios hw/misc/reproducer/simple_reproducer_test.bin

# Check available trace events
./qemu-system-arm --trace help | grep reproducer

# Debug with GDB
./qemu-system-arm -M reproducer -nographic -bios hw/misc/reproducer/simple_reproducer_test.bin -s -S

# Test in monitor:
(qemu) info mtree                  # See memory layout with ROM/RAM
(qemu) x/5i 0x0                   # Disassemble ROM content
(qemu) x/x 0x40000000             # Read trigger device
(qemu) o/w 0x40000000 1           # Write to trigger → resize
(qemu) info mtree                 # See overlap created
```

## Technical Notes

### Memory Region Operations Used

```c
// These functions are used and should be available through proper headers:
memory_region_init_io()
memory_region_init()
memory_region_init_alias()
memory_region_add_subregion()
memory_region_del_subregion()
sysbus_init_mmio()
```

### Build System Pattern

- Uses `ss.source_set()` pattern like other QEMU components
- Properly integrated with trace event generation
- Follows QEMU naming conventions and directory structure

### Code Quality Status

- All function signatures corrected for modern QEMU
- Proper object-oriented structure with TypeInfo
- Error handling with Error **errp parameters
- Follows QEMU device model patterns
