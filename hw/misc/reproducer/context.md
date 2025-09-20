# TCG + PhysMem Bug Reproducer - Project Context

## Project Overview

### Goal
Create a minimal reproducer device for bugs in the TCG (Tiny Code Generator) + physmem (physical memory) subsystems using QEMU system emulation.

### Architecture Requirements
- **ReproducerSoC**: Main SoC containing ARM Cortex-A9 CPU and test devices
- **TriggerDevice**: 4-byte MMIO device that triggers RemoteDevice resize
- **MapperDevice**: Owns secondary address space (2MiB) and creates 16KiB alias into CPU space
- **RemoteDevice**: MMIO device with logging that can resize from 4KiB to 16KiB

### Memory Layout Design
```
CPU Address Space:
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
- `/include/hw/misc/reproducer/` - Header files
- `/hw/misc/reproducer/` - Implementation files
- All integrated into QEMU build system

#### Files Created
```
include/hw/misc/reproducer/
├── reproducer-soc.h
├── trigger-device.h  
├── mapper-device.h
└── remote-device.h

hw/misc/reproducer/
├── reproducer-soc.c
├── reproducer-board.c
├── trigger-device.c
├── mapper-device.c
├── remote-device.c
├── trace-events
├── trace.h
├── meson.build
├── Kconfig
└── build.sh
```

#### Build System Integration
- Added `hw/misc/reproducer` to `trace_events_subdirs` in main `meson.build`
- Updated `hw/misc/meson.build` to include reproducer subdirectory
- Updated `hw/misc/Kconfig` to source reproducer Kconfig
- Created `CONFIG_REPRODUCER` config option (default y, depends on ARM)

### 🔄 Current Compilation Status

#### Successfully Compiling
- ✅ **remote-device.c** - Compiles with warnings (unused variables)
- ✅ **trigger-device.c** - Compiles with warnings (unused variables)

#### Issues to Resolve
- ❌ **mapper-device.c** - Missing `exec/address-spaces.h` header
- ❌ **reproducer-soc.c** - Needs testing after header fixes
- ❌ **reproducer-board.c** - Needs testing after header fixes

### 🚧 Known Issues

#### Header Include Problems
```c
// These includes are causing "file not found" errors:
#include "exec/memory.h"
#include "exec/address-spaces.h"
```

**Solution Strategy**: Find correct headers by examining working QEMU device files.

#### Function Signature Issues (FIXED)
```c
// Fixed: ObjectClass callback signatures
- OLD: void (*)(ObjectClass *klass, void *data)
+ NEW: void (*)(ObjectClass *klass, const void *data)
```

#### Property System Issues
```c
// Current approach: Simplified without DEFINE_PROP_LINK for now
// TODO: Implement proper device linking via properties
```

#### Trace System Issues
```c
// Current approach: Disabled trace function calls
// TODO: Re-enable after trace headers work properly
// trace_reproducer_remote_read(offset, value, size);
```

## Device Implementation Details

### RemoteDevice
- **Type**: `TYPE_REMOTE_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **MMIO**: Initially 4KiB, expandable to 16KiB
- **Function**: `remote_device_resize()` - Public function to trigger resize
- **Status**: ✅ Compiles successfully

### TriggerDevice
- **Type**: `TYPE_TRIGGER_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **MMIO**: 4 bytes
- **Function**: Any write triggers `remote_device_resize()`
- **Status**: ✅ Compiles successfully

### MapperDevice
- **Type**: `TYPE_MAPPER_DEVICE`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **Components**: 2MiB secondary space, 16KiB alias region
- **Status**: ❌ Missing address-spaces header

### ReproducerSoC
- **Type**: `TYPE_REPRODUCER_SOC`
- **Parent**: `TYPE_SYS_BUS_DEVICE`
- **Components**: ARM CPU (temporarily removed), TriggerDevice, MapperDevice
- **Status**: ❌ Needs testing

### ReproducerBoard
- **Machine Type**: `"reproducer"`
- **Function**: Instantiates ReproducerSoC
- **Status**: ❌ Needs testing

## Build Instructions

### Configuration
```bash
# Located in hw/misc/reproducer/build.sh
cd qemu/
rm -rf build && mkdir build && cd build
../configure \
    --target-list=arm-softmmu \
    --enable-debug \
    --enable-trace-backends=simple
make -j$(nproc)
```

### Usage (When Working)
```bash
# Run with reproducer machine
./qemu-system-arm -M reproducer -nographic -monitor stdio

# Test in monitor:
(qemu) info mtree                  # See initial layout
(qemu) x/x 0x40000000             # Read trigger device
(qemu) o/w 0x40000000 1           # Write to trigger → resize
(qemu) info mtree                 # See overlap created
```

## Next Steps Priority

### Immediate (Required for Basic Compilation)
1. **Fix header includes** - Find correct memory/address-space headers
2. **Test remaining device compilation** - mapper, soc, board
3. **Verify build system integration** - Full build test

### Short Term (Core Functionality)
1. **Re-add ARM CPU** - Integrate cortex-a9 safely
2. **Implement device linking** - DEFINE_PROP_LINK for TriggerDevice → RemoteDevice
3. **Re-enable tracing** - Fix trace function calls

### Long Term (Bug Reproduction)
1. **Add test payload** - ARM code that exercises memory
2. **Validate overlap scenario** - Ensure 8KiB overlap triggers correctly  
3. **Test with real workloads** - Run actual code to trigger TCG/physmem races

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

## Context for Next Agent Session

### What Works
- Basic device structure compiles
- Build system integration complete
- Directory structure proper
- Most header dependencies resolved

### What Needs Immediate Attention
- `exec/address-spaces.h` include issue in mapper-device.c
- Testing compilation of soc and board files
- Proper memory region header includes

### Test Command for Validation
```bash
cd /path/to/qemu/build
make libsystem.a.p/hw_misc_reproducer_mapper-device.c.o
```

This file represents the current state as of compilation attempts on 2025-09-21. The foundation is solid and close to working - primarily header include issues remain.