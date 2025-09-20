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
    
    /* TODO: add tracing back later */
    return value;
}

static void remote_device_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    RemoteDeviceState *s = REMOTE_DEVICE(opaque);
    
    (void)s;  /* Suppress unused variable warning */
    (void)offset;
    (void)value;
    (void)size;
    
    /* TODO: add tracing back later */
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
    if (s->is_expanded) {
        return; /* Already expanded */
    }
    
    /* Remove the old region */
    memory_region_del_subregion(s->mmio.container, &s->mmio);
    
    /* Recreate with new size */
    object_unparent(OBJECT(&s->mmio));
    memory_region_init_io(&s->mmio, OBJECT(s), &remote_device_ops, s,
                          "reproducer-remote-mmio", REMOTE_DEVICE_EXPANDED_SIZE);
    
    /* Add back to container */
    memory_region_add_subregion(s->mmio.container, 0, &s->mmio);
    
    s->is_expanded = true;
    
    /* TODO: add tracing back later */
}

static void remote_device_realize(DeviceState *dev, Error **errp)
{
    RemoteDeviceState *s = REMOTE_DEVICE(dev);
    
    memory_region_init_io(&s->mmio, OBJECT(s), &remote_device_ops, s,
                          "reproducer-remote-mmio", REMOTE_DEVICE_INITIAL_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mmio);
    
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
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(RemoteDeviceState),
    .class_init = remote_device_class_init,
};

static void remote_device_register_types(void)
{
    type_register_static(&remote_device_info);
}

type_init(remote_device_register_types)
