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
    @ Write to trigger device to start timer
    ldr r2, =0x40000000      @ Trigger device base address
    ldr r1, =0x12345678      @ Test value 
    str r1, [r2]             @ Write to trigger device - starts timer!
    
    @ Wait for timer to expire (1ms delay) - busy wait loop
    ldr r3, =100000          @ Loop counter for delay
wait_loop:
    subs r3, r3, #1
    bne wait_loop
    
    @ Access through alias region (should trigger assertion)
    ldr r0, =0x50000000      @ ALIAS_REGION_BASE  
    ldr r1, [r0]             @ Read from alias region
    
    @ This access should fail with assertion in iotlb_to_section
    @ because TCG dispatch is stale after timer modified secondary space
    
    /* Infinite loop */
loop:
    b loop
