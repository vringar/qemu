/* Linker script for ARM reproducer test program */

MEMORY
{
    /* ROM region where QEMU loads the test program */
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 16K
}

SECTIONS
{
    .vectors : {
        *(.vectors)
    } > ROM
    
    .text : {
        *(.text)
    } > ROM
    
    .data : {
        *(.data)
    } > ROM
    
    .bss : {
        *(.bss)
    } > ROM
}
