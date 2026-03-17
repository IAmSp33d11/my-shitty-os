BUILD_DIR := build/
SOURCE_DIR := src/
BOOT_DIR := $(SOURCE_DIR)boot/
KERNEL_DIR := $(SOURCE_DIR)kernel/
LIB_DIR := $(SOURCE_DIR)libraries/
DRIVER_DIR := $(SOURCE_DIR)drivers/
OS_NAME := myos
GCC_FLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(SOURCE_DIR)include

all : dirs boot drivers libraries kernel link finish

run:
	qemu-system-i386 -cdrom $(BUILD_DIR)$(OS_NAME).iso -monitor stdio -no-shutdown -no-reboot

finish:
	cp $(BUILD_DIR)$(OS_NAME) $(BUILD_DIR)isodir/boot/$(OS_NAME)
	cp grub.cfg $(BUILD_DIR)isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)$(OS_NAME).iso $(BUILD_DIR)isodir

link:
	i686-elf-gcc -T linker.ld -o $(BUILD_DIR)$(OS_NAME) -ffreestanding -O2 -nostdlib $(BUILD_DIR)boot/* $(BUILD_DIR)libraries/* $(BUILD_DIR)kernel/* $(BUILD_DIR)drivers/* -lgcc

boot:
	i686-elf-as $(BOOT_DIR)boot.s -o $(BUILD_DIR)boot/boot_s.o
	i686-elf-as $(BOOT_DIR)funcs.s -o $(BUILD_DIR)boot/funcs.o
	i686-elf-gcc -c $(BOOT_DIR)isr.c -o $(BUILD_DIR)boot/isr.o $(GCC_FLAGS)
	i686-elf-gcc -c $(BOOT_DIR)boot.c -o $(BUILD_DIR)boot/boot_c.o $(GCC_FLAGS)
	nasm -f elf $(BOOT_DIR)idt.s -o $(BUILD_DIR)boot/idt.o
	

drivers:
	i686-elf-gcc -c $(DRIVER_DIR)vga.c -o $(BUILD_DIR)drivers/vga.o $(GCC_FLAGS)
	i686-elf-gcc -c $(DRIVER_DIR)keyboard.c -o $(BUILD_DIR)drivers/keyboard.o $(GCC_FLAGS)
	i686-elf-gcc -c $(DRIVER_DIR)timing.c -o $(BUILD_DIR)drivers/timing.o $(GCC_FLAGS)

libraries:
	i686-elf-gcc -c $(LIB_DIR)port.c -o $(BUILD_DIR)libraries/port.o $(GCC_FLAGS)
	i686-elf-gcc -c $(LIB_DIR)string.c -o $(BUILD_DIR)libraries/string.o $(GCC_FLAGS)
	i686-elf-gcc -c $(LIB_DIR)paging.c -o $(BUILD_DIR)libraries/paging.o $(GCC_FLAGS)
	i686-elf-gcc -c $(LIB_DIR)disc.c -o $(BUILD_DIR)libraries/disc.o $(GCC_FLAGS)

kernel:
	i686-elf-gcc -c $(KERNEL_DIR)kernel.c -o $(BUILD_DIR)kernel/kernel.o $(GCC_FLAGS)

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(SOURCE_DIR)
	mkdir -p $(BUILD_DIR)kernel/
	mkdir -p $(BUILD_DIR)boot/
	mkdir -p $(BUILD_DIR)isodir/boot/grub/
	mkdir -p $(BUILD_DIR)libraries/
	mkdir -p $(BUILD_DIR)drivers/

.PHONY : clean
clean:
	rm -r $(BUILD_DIR)*