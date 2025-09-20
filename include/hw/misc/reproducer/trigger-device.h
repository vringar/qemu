/*
 * TCG + PhysMem Bug Reproducer - Trigger Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef HW_MISC_REPRODUCER_TRIGGER_DEVICE_H
#define HW_MISC_REPRODUCER_TRIGGER_DEVICE_H

#include "qom/object.h"
#include "hw/sysbus.h"
#include "hw/misc/reproducer/remote-device.h"

#define TYPE_TRIGGER_DEVICE "reproducer-trigger-device"
OBJECT_DECLARE_SIMPLE_TYPE(TriggerDeviceState, TRIGGER_DEVICE)

struct TriggerDeviceState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    RemoteDeviceState *remote_device;
};

#endif /* HW_MISC_REPRODUCER_TRIGGER_DEVICE_H */
