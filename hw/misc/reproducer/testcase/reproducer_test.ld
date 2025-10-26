/* Linker script for ARM reproducer test program (C version) */

MEMORY
{
    /* ROM region where QEMU loads the test program */
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 16K
    /* RAM region for runtime data */
    RAM (rw) : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Entry point */
ENTRY(_start)

SECTIONS
{
    /* Reset vector at beginning of ROM */
    .vectors : {
        KEEP(*(.vectors))
        . = ALIGN(4);
    } > ROM
    
    /* Program code */
    .text : {
        *(.text.startup)
        *(.text._start)
        *(.text.main)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > ROM
    
    /* Initialized data - loaded in ROM but copied to RAM at runtime */
    .data : {
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM AT > ROM
    
    /* Uninitialized data in RAM */
    .bss : {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM
    
    /* Stack pointer setup */
    .stack : {
        . = ALIGN(8);
        __stack_start__ = .;
        . += 1024; /* 1KB stack */
        __stack_end__ = .;
    } > RAM
    
    /* Provide stack top symbol */
    PROVIDE(__stack_top__ = __stack_end__);
    
    /* Discard debug info to keep binary small */
    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.note*)
        *(.comment*)
        *(.debug*)
    }
}
