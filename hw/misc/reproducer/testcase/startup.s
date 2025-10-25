/*
 * Startup code for ARM reproducer test (C version)
 */

.syntax unified
.thumb

/* Vector table */
.section .vectors, "a"
.global _vectors
_vectors:
    .word __stack_top__     /* Initial stack pointer */
    .word _start + 1        /* Reset handler (Thumb mode) */

/* Reset handler */
.section .text.startup, "ax"
.global _start
.thumb_func
_start:
    /* Set up stack pointer */
    ldr r0, =__stack_top__
    mov sp, r0
    
    /* Zero BSS section */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
bss_loop:
    cmp r0, r1
    bge bss_done
    str r2, [r0]
    add r0, r0, #4
    b bss_loop
bss_done:

    /* Copy data section from ROM to RAM (if needed) */
    ldr r0, =__data_start__
    ldr r1, =__data_end__
    ldr r2, =__data_start__
    cmp r0, r2
    beq data_done
data_loop:
    cmp r0, r1
    bge data_done
    ldr r3, [r2]
    str r3, [r0]
    add r0, r0, #4
    add r2, r2, #4
    b data_loop
data_done:

    /* Call main function */
    bl main
    
    /* If main returns, infinite loop */
infinite_loop:
    wfi                     /* Wait for interrupt */
    b infinite_loop

.end
