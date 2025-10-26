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
#include "qemu/timer.h"

/* Global pointer to access device from timer callback - this simulates
 * the real-world scenario where device is modified from fuzzer callback */
static TriggerDeviceState *global_trigger_device_state = NULL;

/* Timer callback that modifies secondary address space WITHOUT triggering
 * primary address space commit - this is the key to reproducing the bug! */
static void write_catcher_resize_callback(void *opaque)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(opaque);
    
    if (!s->remote_device || s->timer_triggered) {
        return;
    }
    
    trace_reproducer_trigger_device_timer_resize();
    
    /* THIS IS THE BUG: Directly modify the secondary address space memory region
     * WITHOUT going through the normal device resize that would trigger commits.
     * This causes the TCG dispatch tables to become stale! */
    memory_region_transaction_begin();
    
    /* Get direct access to the remote device's memory region and modify it
     * This simulates the write_catcher_activate() function from the blog post */
    MemoryRegion *remote_mmio = &s->remote_device->mmio;
    
    /* Change the size of the remote device region directly - this should 
     * cause the dispatch map to become inconsistent */
    memory_region_set_size(remote_mmio, 32 * 1024);  /* Expand from 4KiB to 32KiB */
    
    memory_region_transaction_commit();
    
    s->timer_triggered = true;
    trace_reproducer_trigger_device_timer_complete();
}

static uint64_t trigger_device_read(void *opaque, hwaddr offset, unsigned size)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(opaque);
    
    (void)s;      /* Suppress unused variable warning */
    (void)offset; /* Suppress unused parameter warning */
    (void)size;   /* Suppress unused parameter warning */
    
    /* TODO: add tracing back later */
    return s->trigger_value; /* Always return 0 */
}

static void trigger_device_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    TriggerDeviceState *s = TRIGGER_DEVICE(opaque);
    
    trace_reproducer_trigger_device_write(offset, value, size);
    s->trigger_value = value;
    
    /* Start timer to modify secondary address space asynchronously - this is the key! */
    if (s->remote_device && !s->timer_triggered) {
        trace_reproducer_trigger_device_resize_triggered();
        /* Delay the modification slightly to ensure it happens outside primary AS context */
        timer_mod(s->resize_timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + 1000000); /* 1ms delay */
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
                          TYPE_TRIGGER_DEVICE, TRIGGER_DEVICE_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mmio);
    
    /* Initialize write catcher device (initially small and unmapped) */
    memory_region_init_io(&s->write_catcher, OBJECT(s), &trigger_device_ops, s,
                          "reproducer-write-catcher", 0x1000);
    
    /* Create timer for asynchronous address space modification */
    s->resize_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, write_catcher_resize_callback, s);
    s->timer_triggered = false;
    
    /* Set global pointer for timer callback access */
    global_trigger_device_state = s;
}

static Property trigger_device_properties[] = {
    DEFINE_PROP_LINK("remote-device", TriggerDeviceState, remote_device,
                     TYPE_REMOTE_DEVICE, RemoteDeviceState *),
};

static void trigger_device_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    
    dc->realize = trigger_device_realize;
    dc->desc = "Reproducer Trigger Device";
    device_class_set_props(dc, trigger_device_properties);
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

