/*
 * TCG + PhysMem Bug Reproducer SoC
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/misc/reproducer/reproducer-soc.h"
#include "hw/qdev-properties.h"
#include "system/memory.h"

static void reproducer_soc_init(Object *obj)
{
    ReproducerSocState *s = REPRODUCER_SOC(obj);
    
    /* We'll just create a simple SoC without CPU for now to get it compiling */
    object_initialize_child(obj, "trigger", &s->trigger, TYPE_TRIGGER_DEVICE);
    object_initialize_child(obj, "mapper", &s->mapper, TYPE_MAPPER_DEVICE);
}

static void reproducer_soc_realize(DeviceState *dev, Error **errp)
{
    ReproducerSocState *s = REPRODUCER_SOC(dev);
    SysBusDevice *sysbusdev;
    
    /* Realize the mapper device first (it contains the remote device) */
    sysbus_realize_and_unref(SYS_BUS_DEVICE(&s->mapper), errp);
    if (*errp) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->mapper);
    sysbus_mmio_map(sysbusdev, 0, REPRODUCER_MAPPER_BASE);
    
    /* Set up the link between trigger and remote device */
    object_property_set_link(OBJECT(&s->trigger), "remote-device",
                             OBJECT(&s->mapper.remote_device), &error_fatal);
    
    /* Realize the trigger device */
    sysbus_realize_and_unref(SYS_BUS_DEVICE(&s->trigger), errp);
    if (*errp) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->trigger);
    sysbus_mmio_map(sysbusdev, 0, REPRODUCER_TRIGGER_BASE);
}

static void reproducer_soc_class_init(ObjectClass *oc, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    
    dc->realize = reproducer_soc_realize;
    dc->desc = "Reproducer SoC";
    dc->user_creatable = false;
}

static const TypeInfo reproducer_soc_info = {
    .name = TYPE_REPRODUCER_SOC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(ReproducerSocState),
    .instance_init = reproducer_soc_init,
    .class_init = reproducer_soc_class_init,
};

static void reproducer_soc_register_types(void)
{
    type_register_static(&reproducer_soc_info);
}

type_init(reproducer_soc_register_types)
