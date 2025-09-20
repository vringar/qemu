/*
 * TCG + PhysMem Bug Reproducer - Mapper Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef HW_MISC_REPRODUCER_MAPPER_DEVICE_H
#define HW_MISC_REPRODUCER_MAPPER_DEVICE_H

#include "qom/object.h"
#include "hw/sysbus.h"
#include "hw/misc/reproducer/remote-device.h"

#define TYPE_MAPPER_DEVICE "reproducer-mapper-device"
OBJECT_DECLARE_SIMPLE_TYPE(MapperDeviceState, MAPPER_DEVICE)

#define MAPPER_SECONDARY_SPACE_SIZE  (2 * 1024 * 1024)  /* 2 MiB */
#define MAPPER_ALIAS_SIZE            (16 * 1024)        /* 16 KiB */
#define MAPPER_ALIAS_OFFSET          0x2000              /* Offset in secondary space */

struct MapperDeviceState {
    SysBusDevice parent_obj;

    MemoryRegion secondary_space;
    MemoryRegion alias_region;
    AddressSpace secondary_as;
    
    RemoteDeviceState remote_device;
};

#endif /* HW_MISC_REPRODUCER_MAPPER_DEVICE_H */
