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
#include "qemu/timer.h"

#define TYPE_TRIGGER_DEVICE "reproducer-trigger-device"
OBJECT_DECLARE_SIMPLE_TYPE(TriggerDeviceState, TRIGGER_DEVICE)

#define TRIGGER_DEVICE_SIZE 0x1000

struct TriggerDeviceState {
    SysBusDevice parent_obj;
    int64_t trigger_value;
    MemoryRegion mmio;
    MemoryRegion write_catcher;  /* Device that will be resized in secondary space */
    RemoteDeviceState *remote_device;
    QEMUTimer *resize_timer;     /* Timer for async address space modification */
    bool timer_triggered;
};

#endif /* HW_MISC_REPRODUCER_TRIGGER_DEVICE_H */
