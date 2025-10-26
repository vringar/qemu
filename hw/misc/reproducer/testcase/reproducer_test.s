/*
 * ARM Assembly Test Program for TCG + PhysM    /* Test the alias range after resize */
    ldr r0, =0x40001000         @ Alias base (maps to secondary 0x2000+)
    ldr r1, =0xDEADBEEF         @ First test pattern
    str r1, [r0]                @ Write to 0x40001000 -> secondary 0x2000
    
    /* Write to the overlap region */
    ldr r0, =0x40002000         @ Middle of alias (secondary 0x3000)  
    ldr r1, =0xCAFEBABE         @ Second test pattern
    str r1, [r0]                @ This should hit the overlapped region
    
    /* Test edge of overlap */
    ldr r0, =0x40002FFC         @ Near end of overlap (secondary 0x3FFC)
    ldr r1, =0xBEEFCAFE         @ Third test pattern
    str r1, [r0]                @ Last 4 bytes of overlapcer
 *
 * This program:
 * 1. Sets up proper ARM vector table
 * 2. Writes to the trigger device (0x40000000) to resize the remote device  
 * 3. Writes through the alias (0x40001000+) to test the overlap scenario
 * 4. Creates extensive trace output for debugging
 */

.syntax unified
.cpu cortex-a9
.text

/* Vector table for ARM - must be at start of ROM */
.section .vectors, "ax"
.global _start

_start:
    ldr pc, =reset_handler       @ Reset vector
    ldr pc, =undefined_handler   @ Undefined instruction
    ldr pc, =swi_handler         @ Software interrupt  
    ldr pc, =prefetch_handler    @ Prefetch abort
    ldr pc, =data_handler        @ Data abort
    ldr pc, =reserved_handler    @ Reserved
    ldr pc, =irq_handler         @ IRQ
    ldr pc, =fiq_handler         @ FIQ

/* Main program starts here */
.section .text
.global reset_handler

reset_handler:
    /* Set up stack pointer (use end of RAM) */
    ldr sp, =0x80000
    
    /* Phase 1: Initial state - write to trigger device */
    ldr r0, =0x40000000         @ Trigger device base address
    ldr r1, =0x12345678         @ Test value to write  
    str r1, [r0]                @ Write to trigger device (should resize remote)
    
    /* Small delay to ensure resize completes */
    mov r2, #10000
delay1:
    subs r2, r2, #1
    bne delay1
    
    /* Phase 2: Test the alias range after resize */
    ldr r0, =0x40001000         @ Alias base (maps to secondary 0x2000+)
    mov r1, #0xDEADBEEF         @ First test pattern
    str r1, [r0]                @ Write to 0x40001000 -> secondary 0x2000
    
    /* Write to the overlap region */
    ldr r0, =0x40002000         @ Middle of alias (secondary 0x3000)  
    mov r1, #0xCAFEBABE         @ Second test pattern
    str r1, [r0]                @ This should hit the overlapped region
    
    /* Test edge of overlap */
    ldr r0, =0x40002FFC         @ Near end of overlap (secondary 0x3FFC)
    mov r1, #0xBEEFCAFE         @ Third test pattern
    str r1, [r0]                @ Last 4 bytes of overlap
    
    /* Phase 3: Read back through alias to verify */
    ldr r0, =0x40001000
    ldr r3, [r0]                @ Read back first write
    
    ldr r0, =0x40002000  
    ldr r4, [r0]                @ Read back overlap write
    
    ldr r0, =0x40002FFC
    ldr r5, [r0]                @ Read back edge write
    
    /* Phase 4: Access beyond overlap to test boundaries */
    ldr r0, =0x40003000         @ Beyond overlap (secondary 0x4000)
    ldr r1, =0x11111111         @ Boundary test pattern
    str r1, [r0]                @ Should access alias but not remote device
    
    ldr r0, =0x40004000         @ Far beyond (secondary 0x5000)
    ldr r1, =0x22222222         @ Second boundary pattern
    str r1, [r0]                @ Another boundary test
    
    /* Phase 5: Multiple trigger attempts */
    ldr r0, =0x40000000         @ Trigger device again
    ldr r1, =0x87654321         @ Different trigger value
    str r1, [r0]                @ Should be ignored (already resized)
    
    mov r2, #1000
delay2:
    subs r2, r2, #1
    bne delay2
    
    /* Phase 6: Final verification reads */
    ldr r0, =0x40001000
    ldr r6, [r0]                @ Final read of overlap start
    
    ldr r0, =0x40002000
    ldr r7, [r0]                @ Final read of overlap middle
    
    ldr r0, =0x40003000  
    ldr r8, [r0]                @ Final read beyond overlap
    
    /* Phase 7: Pattern writes for memory content verification */
    ldr r0, =0x40001000         @ Start of alias
    mov r1, #0x00
    mov r2, #256                @ Write 256 words of incrementing pattern
pattern_loop:
    str r1, [r0], #4            @ Store and increment address
    add r1, r1, #1              @ Increment pattern
    subs r2, r2, #1
    bne pattern_loop

success_loop:
    /* Signal success and loop forever for observation */
    b success_loop

/* Exception handlers - simple loops for debugging */
undefined_handler:
    mov r12, #0x1               @ Load error code for debugging
    b exception_loop

swi_handler:
    mov r12, #0x2
    b exception_loop
    
prefetch_handler:
    mov r12, #0x3
    b exception_loop
    
data_handler:
    mov r12, #0x4  
    b exception_loop
    
reserved_handler:
    mov r12, #0x5
    b exception_loop
    
irq_handler:
    mov r12, #0x6
    b exception_loop
    
fiq_handler:
    mov r12, #0x7
    b exception_loop

exception_loop:
    /* Infinite loop with error code in r12 */
    b exception_loop

.end
