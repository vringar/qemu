/*
 * TCG + PhysMem Bug Reproducer - Trigger Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/misc/reproducer/trigger-device.h"
#include "hw/qdev-properties.h"
#include "trace.h"

static uint64_t trigger_device_read(void *opaque, hwaddr offset, unsigned size)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(opaque);
    
    (void)s;      /* Suppress unused variable warning */
    (void)offset; /* Suppress unused parameter warning */
    (void)size;   /* Suppress unused parameter warning */
    
    /* TODO: add tracing back later */
    return 0; /* Always return 0 */
}

static void trigger_device_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(opaque);
    
    /* TODO: add tracing back later */
    
    /* Any write triggers the remote device resize */
    if (s->remote_device) {
        remote_device_resize(s->remote_device);
    }
}

static const MemoryRegionOps trigger_device_ops = {
    .read = trigger_device_read,
    .write = trigger_device_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
};

static void trigger_device_realize(DeviceState *dev, Error **errp)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(dev);
    
    memory_region_init_io(&s->mmio, OBJECT(s), &trigger_device_ops, s,
                          "reproducer-trigger-mmio", 4);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mmio);
}

static void trigger_device_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    
    dc->realize = trigger_device_realize;
    dc->desc = "Reproducer Trigger Device";
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo trigger_device_info = {
    .name = TYPE_TRIGGER_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(TriggerDeviceState),
    .class_init = trigger_device_class_init,
};

static void trigger_device_register_types(void)
{
    type_register_static(&trigger_device_info);
}

type_init(trigger_device_register_types)
