/*
 * TCG + physmem bug reproducer test in C
 * 
 * This reproducer aims to trigger the TCG dispatch staleness bug described in:
 * http://127.0.0.1:1111/posts/master/anatomy-of-a-bug/
 * 
 * The bug occurs when:
 * 1. A timer modifies a secondary address space asynchronously
 * 2. The TCG dispatch map becomes stale without triggering commit handlers
 * 3. CPU access through alias region triggers assertion in iotlb_to_section()
 */

// Memory map definitions
#define TRIGGER_DEVICE_BASE  0x40000000
#define ALIAS_REGION_BASE    0x50000000

// Device registers
#define TRIGGER_REG_OFFSET   0x0

// Simple volatile pointer access functions
static inline void write32(unsigned long addr, unsigned int value)
{
    *((volatile unsigned int *)addr) = value;
}

static inline unsigned int read32(unsigned long addr)
{
    return *((volatile unsigned int *)addr);
}

// Simple delay function
static void delay(unsigned int cycles)
{
    volatile unsigned int i;
    for (i = 0; i < cycles; i++) {
        // Simple busy wait
        asm volatile("nop");
    }
}

// Main test function
int main(void)
{
    // Step 1: Write to trigger device to start timer-based secondary space modification
    // This triggers a timer that will modify the secondary address space asynchronously
    write32(TRIGGER_DEVICE_BASE + TRIGGER_REG_OFFSET, 0x12345678);
    
    // Step 2: Wait for timer to execute and modify secondary address space
    // The timer should fire and call memory_region_set_size() directly
    // without triggering TCG commit handlers, making dispatch map stale
    delay(1000000);  // Longer delay to ensure timer fires
    
    // Step 3: Access through alias region to trigger assertion
    // This access should go through stale TCG dispatch map and trigger:
    // assert(section_index < d->map.sections_nb) in iotlb_to_section()
    volatile unsigned int test_value = read32(ALIAS_REGION_BASE);
    
    // If we reach here, the assertion didn't trigger
    // Return the value to prevent optimization
    return test_value;
}
