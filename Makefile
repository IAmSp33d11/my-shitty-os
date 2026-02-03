BUILD_DIR := build/
SOURCE_DIR := src/
BOOT_DIR := $(SOURCE_DIR)boot/
KERNEL_DIR := $(SOURCE_DIR)kernel/
LIB_DIR := $(SOURCE_DIR)libraries/
OS_NAME := myos

all : dirs boot libraries kernel link finish

run:
	qemu-system-i386 -cdrom $(BUILD_DIR)$(OS_NAME).iso -monitor stdio -no-shutdown -no-reboot

finish:
	cp $(BUILD_DIR)$(OS_NAME) $(BUILD_DIR)isodir/boot/$(OS_NAME)
	cp grub.cfg $(BUILD_DIR)isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)$(OS_NAME).iso $(BUILD_DIR)isodir

link:
	i686-elf-gcc -T linker.ld -o $(BUILD_DIR)$(OS_NAME) -ffreestanding -O2 -nostdlib $(BUILD_DIR)boot/boot_s.o $(BUILD_DIR)boot/funcs.o $(BUILD_DIR)boot/idt.o $(BUILD_DIR)boot/boot_c.o $(BUILD_DIR)/libraries/port.o $(BUILD_DIR)kernel/kernel.o -lgcc

boot:
	i686-elf-as $(BOOT_DIR)boot.s -o $(BUILD_DIR)boot/boot_s.o
	i686-elf-as $(BOOT_DIR)funcs.s -o $(BUILD_DIR)boot/funcs.o
	i686-elf-gcc -c $(BOOT_DIR)boot.c -o $(BUILD_DIR)boot/boot_c.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	nasm -f elf $(BOOT_DIR)idt.s -o $(BUILD_DIR)boot/idt.o

libraries:
	i686-elf-gcc -c $(LIB_DIR)port.c -o $(BUILD_DIR)libraries/port.o
kernel:
	i686-elf-gcc -c $(KERNEL_DIR)kernel.c -o $(BUILD_DIR)kernel/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(SOURCE_DIR)
	mkdir -p $(BUILD_DIR)kernel/
	mkdir -p $(BUILD_DIR)boot/
	mkdir -p $(BUILD_DIR)isodir/boot/grub/
	mkdir -p $(BUILD_DIR)libraries/

.PHONY : clean
clean:
	rm -r $(BUILD_DIR)