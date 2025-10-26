/*
 * TCG + PhysMem Bug Reproducer - Remote Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef HW_MISC_REPRODUCER_REMOTE_DEVICE_H
#define HW_MISC_REPRODUCER_REMOTE_DEVICE_H

#include "qom/object.h"
#include "hw/qdev-core.h"
#include "system/memory.h"

#define TYPE_REMOTE_DEVICE "reproducer-remote-device"
OBJECT_DECLARE_SIMPLE_TYPE(RemoteDeviceState, REMOTE_DEVICE)

#define REMOTE_DEVICE_INITIAL_SIZE  (4 * 1024)   /* 4 KiB */
#define REMOTE_DEVICE_EXPANDED_SIZE (16 * 1024)  /* 16 KiB */

struct RemoteDeviceState {
    DeviceState parent_obj;

    MemoryRegion mmio;
    bool is_expanded;
};

/* Public function to resize the device */
void remote_device_resize(RemoteDeviceState *s);

#endif /* HW_MISC_REPRODUCER_REMOTE_DEVICE_H */
