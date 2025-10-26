# Makefile for TCG + PhysMem Bug Reproducer Test Programs
#
# This Makefile builds two separate test binaries:
# 1. simple_reproducer_test.bin - Assembly version
# 2. reproducer_test.bin - C version with startup code
#

# Cross-compiler setup
ARM_TOOLCHAIN_PREFIX := arm-none-eabi-
ARM_CC := $(ARM_TOOLCHAIN_PREFIX)gcc
ARM_AS := $(ARM_TOOLCHAIN_PREFIX)as
ARM_LD := $(ARM_TOOLCHAIN_PREFIX)ld
ARM_OBJCOPY := $(ARM_TOOLCHAIN_PREFIX)objcopy

# Compiler and linker flags
ASFLAGS := 
CFLAGS := -mthumb -mcpu=cortex-a9 -O1 -nostdlib -nostartfiles -ffreestanding \
          -fno-builtin -fno-stack-protector -Wall -Wextra
LDFLAGS := -T reproducer_test.ld

# Build targets
TARGETS := simple_reproducer_test.bin reproducer_test.bin
OBJECTS := simple_reproducer_test.o startup.o reproducer_test.o
ELFS := simple_reproducer_test.elf reproducer_test.elf

# Default target
.PHONY: all clean help
all: $(TARGETS)

# Assembly version - single file
simple_reproducer_test.bin: simple_reproducer_test.elf
	$(ARM_OBJCOPY) -O binary $< $@

simple_reproducer_test.elf: simple_reproducer_test.o reproducer_test.ld
	$(ARM_LD) $(LDFLAGS) -o $@ $<

simple_reproducer_test.o: simple_reproducer_test.s
	$(ARM_AS) $(ASFLAGS) -o $@ $<

# C version - with startup code
reproducer_test.bin: reproducer_test.elf
	$(ARM_OBJCOPY) -O binary $< $@

reproducer_test.elf: startup.o reproducer_test.o reproducer_test.ld
	$(ARM_LD) $(LDFLAGS) -o $@ startup.o reproducer_test.o

startup.o: startup.s
	$(ARM_CC) -c $(CFLAGS) -o $@ $<

reproducer_test.o: reproducer_test.c
	$(ARM_CC) -c $(CFLAGS) -o $@ $<

# Utility targets
.PHONY: test test-new
test: $(TARGETS)
	@echo "Running tests with assembly version..."
	./test_reproducer.sh

test-new: $(TARGETS)
	@echo "Running tests with C version..."
	./test_reproducer.sh --new

clean:
	rm -f $(OBJECTS) $(ELFS) $(TARGETS)

help:
	@echo "Available targets:"
	@echo "  all                    - Build both test binaries"
	@echo "  simple_reproducer_test.bin - Assembly version"
	@echo "  reproducer_test.bin    - C version"
	@echo "  clean                  - Remove all build artifacts"
	@echo "  help                   - Show this help message"
	@echo "  check-tools            - Verify ARM cross-compiler availability"
	@echo ""
	@echo "Test scripts:"
	@echo "  ./test_reproducer.sh [--new]     - Run functionality tests"
	@echo "  ./debug_reproducer.sh [--new]    - Run debug session with GDB"
	@echo "  --new flag uses C version (reproducer_test.bin) instead of assembly"

# Dependency tracking
simple_reproducer_test.o: simple_reproducer_test.s
startup.o: startup.s
reproducer_test.o: reproducer_test.c
simple_reproducer_test.elf: reproducer_test.ld
reproducer_test.elf: reproducer_test.ld

# Check for required tools
.PHONY: check-tools
check-tools:
	@which $(ARM_CC) >/dev/null || (echo "ERROR: $(ARM_CC) not found. Please install ARM cross-compiler." && exit 1)
	@which $(ARM_AS) >/dev/null || (echo "ERROR: $(ARM_AS) not found. Please install ARM cross-compiler." && exit 1)
	@which $(ARM_LD) >/dev/null || (echo "ERROR: $(ARM_LD) not found. Please install ARM cross-compiler." && exit 1)
	@which $(ARM_OBJCOPY) >/dev/null || (echo "ERROR: $(ARM_OBJCOPY) not found. Please install ARM cross-compiler." && exit 1)
	@echo "All required tools found."

# Add check-tools as a prerequisite for build targets
$(OBJECTS): | check-tools
