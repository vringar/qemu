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
#include "target/arm/cpu-qom.h"
#include "hw/loader.h"
#include "qemu/units.h"
#include "hw/core/cpu.h"

#define REPRODUCER_BIOS_ADDR    0x00000000

static void reproducer_board_init(MachineState *machine)
{
    DeviceState *soc;
    Object *cpuobj;
    int firmware_size;
    
    if (machine->ram_size != 0) {
        error_report("This board uses fixed memory layout, do not specify -m");
        exit(1);
    }
    
    /* Create the ARM Cortex-A9 CPU */
    cpuobj = object_new(machine->cpu_type);
    
    /* Disable EL3 for simplicity (many ARM CPUs have it enabled by default) */
    if (object_property_find(cpuobj, "has_el3")) {
        object_property_set_bool(cpuobj, "has_el3", false, &error_fatal);
    }
    
    /* Realize the CPU */
    qdev_realize(DEVICE(cpuobj), NULL, &error_fatal);
    
    /* Create and realize the SoC */
    soc = qdev_new(TYPE_REPRODUCER_SOC);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(soc), &error_fatal);
    
    /* Load BIOS/firmware if specified */
    if (machine->firmware) {
        firmware_size = load_image_targphys(machine->firmware, 
                                          REPRODUCER_BIOS_ADDR,
                                          1 * MiB); /* Allow up to 1MB BIOS */
        if (firmware_size < 0) {
            error_report("Could not load BIOS '%s'", machine->firmware);
            exit(1);
        }
        
        /* Set CPU to start from BIOS address */
        cpu_set_pc(CPU(cpuobj), REPRODUCER_BIOS_ADDR);
    }
}

static void reproducer_board_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    
    mc->desc = "TCG + PhysMem Bug Reproducer Board";
    mc->init = reproducer_board_init;
    mc->max_cpus = 1;
    mc->default_cpus = 1;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a9");
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
