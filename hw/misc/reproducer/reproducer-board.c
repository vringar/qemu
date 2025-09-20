/*
 * TCG + PhysMem Bug Reproducer Board
 *
 * Copyright (c) 2025
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/misc/reproducer/reproducer-soc.h"
#include "qemu/error-report.h"

static void reproducer_board_init(MachineState *machine)
{
    DeviceState *soc;
    
    if (machine->ram_size != 0) {
        error_report("This board uses fixed memory layout, do not specify -m");
        exit(1);
    }
    
    soc = qdev_new(TYPE_REPRODUCER_SOC);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(soc), &error_fatal);
}

static void reproducer_board_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    
    mc->desc = "TCG + PhysMem Bug Reproducer Board";
    mc->init = reproducer_board_init;
    mc->max_cpus = 1;
    mc->default_cpus = 1;
    mc->no_floppy = 1;
    mc->no_cdrom = 1;
    /* Note: no_sdcard field removed in newer QEMU versions */
    mc->default_ram_size = 0; /* We manage our own memory */
}

static const TypeInfo reproducer_board_info = {
    .name = MACHINE_TYPE_NAME("reproducer"),
    .parent = TYPE_MACHINE,
    .class_init = reproducer_board_class_init,
};

static void reproducer_machine_init(void)
{
    type_register_static(&reproducer_board_info);
}

type_init(reproducer_machine_init)
