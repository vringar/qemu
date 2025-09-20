/*
 * TCG + PhysMem Bug Reproducer SoC
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef HW_MISC_REPRODUCER_SOC_H
#define HW_MISC_REPRODUCER_SOC_H

#include "qom/object.h"
#include "hw/sysbus.h"
#include "hw/misc/reproducer/trigger-device.h"
#include "hw/misc/reproducer/mapper-device.h"

#define TYPE_REPRODUCER_SOC "reproducer-soc"
OBJECT_DECLARE_SIMPLE_TYPE(ReproducerSocState, REPRODUCER_SOC)

/* Memory map */
#define REPRODUCER_TRIGGER_BASE   0x40000000
#define REPRODUCER_TRIGGER_SIZE   4
#define REPRODUCER_MAPPER_BASE    0x40001000
#define REPRODUCER_MAPPER_SIZE    (16 * 1024)

struct ReproducerSocState {
    SysBusDevice parent_obj;

    DeviceState cpu;
    TriggerDeviceState trigger;
    MapperDeviceState mapper;
};

#endif /* HW_MISC_REPRODUCER_SOC_H */
