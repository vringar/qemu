/*
 * TCG + PhysMem Bug Reproducer - Mapper Device
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/misc/reproducer/mapper-device.h"
#include "hw/qdev-properties.h"
#include "trace.h"

static void mapper_device_realize(DeviceState *dev, Error **errp)
{
    MapperDeviceState *s = MAPPER_DEVICE(dev);
    
    /* Initialize the 2MiB secondary address space */
    memory_region_init(&s->secondary_space, OBJECT(s), 
                       "reproducer-secondary-space", MAPPER_SECONDARY_SPACE_SIZE);
    address_space_init(&s->secondary_as, &s->secondary_space, "reproducer-secondary");
    
    /* Initialize the remote device */
    object_initialize_child(OBJECT(s), "remote-device", &s->remote_device, TYPE_REMOTE_DEVICE);
    if (!qdev_realize(DEVICE(&s->remote_device), NULL, errp)) {
        return;
    }
    
    /* Mount the remote device at offset 0 in secondary space */
    memory_region_add_subregion(&s->secondary_space, 0, &s->remote_device.mmio);
    trace_reproducer_mapper_device_remote_mounted(0, REMOTE_DEVICE_INITIAL_SIZE);
    
    /* Create alias region that maps part of secondary space into CPU space */
    memory_region_init_alias(&s->alias_region, OBJECT(s), 
                             "reproducer-alias", &s->secondary_space,
                             MAPPER_ALIAS_OFFSET, MAPPER_ALIAS_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->alias_region);
    
    /* TODO: add tracing back later */
}

static void mapper_device_unrealize(DeviceState *dev)
{
    MapperDeviceState *s = MAPPER_DEVICE(dev);
    
    address_space_destroy(&s->secondary_as);
}

static void mapper_device_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    
    dc->realize = mapper_device_realize;
    dc->unrealize = mapper_device_unrealize;
    dc->desc = "Reproducer Mapper Device";
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo mapper_device_info = {
    .name = TYPE_MAPPER_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(MapperDeviceState),
    .class_init = mapper_device_class_init,
};

static void mapper_device_register_types(void)
{
    type_register_static(&mapper_device_info);
}

type_init(mapper_device_register_types)
