/*
 * Simple ARM Test Program for TCG + PhysMem Bug Reproducer
 *
 * This program performs just the essential operations:
 * 1. Write to trigger device (0x40000000) to resize the remote device
 * 2. Write to mapper device (0x40001000) through the alias
 * 3. Read from mapper device (0x40001000) to verify the operation
 */

.syntax unified
.cpu cortex-a9
.text

.global _start
_start:
    /* Write to trigger device to resize remote device */
    ldr r0, =0x40000000         @ REPRODUCER_TRIGGER_BASE
    ldr r1, =0x12345678         @ Test value
    str r1, [r0]                @ Trigger resize operation
    
    /* Write to mapper device through alias */
    ldr r0, =0x40001000         @ REPRODUCER_MAPPER_BASE  
    ldr r1, =0xDEADBEEF         @ Test pattern
    str r1, [r0]                @ Write through alias
    
    /* Read back from mapper device */
    ldr r2, [r0]                @ Read back the value
    
    /* Infinite loop */
loop:
    b loop
