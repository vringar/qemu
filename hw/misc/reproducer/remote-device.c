/*
 * TCG + PhysMem Bug Reproducer - Remote Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/misc/reproducer/remote-device.h"
#include "hw/qdev-properties.h"
#include "trace.h"

static uint64_t remote_device_read(void *opaque, hwaddr offset, unsigned size)
{
    RemoteDeviceState *s = REMOTE_DEVICE(opaque);
    uint64_t value = 0;
    
    (void)s;  /* Suppress unused variable warning */
    
    /* Simple pattern: return offset value for easy identification */
    value = offset | (size << 16);
    
    trace_reproducer_remote_device_read(offset, value, size);
    return value;
}

static void remote_device_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    RemoteDeviceState *s = REMOTE_DEVICE(opaque);
    
    (void)s;  /* Suppress unused variable warning */
    
    trace_reproducer_remote_device_write(offset, value, size);
    /* No actual functionality - just logging */
}

static const MemoryRegionOps remote_device_ops = {
    .read = remote_device_read,
    .write = remote_device_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
};

void remote_device_resize(RemoteDeviceState *s)
{
    trace_reproducer_remote_device_resize_start(s->is_expanded);
    
    if (s->is_expanded) {
        trace_reproducer_remote_device_resize_skipped();
        return;
    }
    
    /* Expand from 4KiB to 16KiB */
    uint32_t old_size = REMOTE_DEVICE_INITIAL_SIZE;
    uint32_t new_size = REMOTE_DEVICE_EXPANDED_SIZE;
    
    memory_region_set_size(&s->mmio, new_size);
    s->is_expanded = true;
    
    trace_reproducer_remote_device_resized(old_size, new_size);
}

static void remote_device_realize(DeviceState *dev, Error **errp)
{
    RemoteDeviceState *s = REMOTE_DEVICE(dev);
    
    memory_region_init_io(&s->mmio, OBJECT(s), &remote_device_ops, s,
                          "reproducer-remote-mmio", REMOTE_DEVICE_INITIAL_SIZE);
    
    s->is_expanded = false;
}

static void remote_device_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    
    dc->realize = remote_device_realize;
    dc->desc = "Reproducer Remote Device";
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo remote_device_info = {
    .name = TYPE_REMOTE_DEVICE,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(RemoteDeviceState),
    .class_init = remote_device_class_init,
};

static void remote_device_register_types(void)
{
    type_register_static(&remote_device_info);
}

type_init(remote_device_register_types)
