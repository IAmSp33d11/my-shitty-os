/* Declare constants for the multiboot2 header. */
.set MAGIC,    0xE85250D6 /* 'magic number' lets bootloader find the header */
.set ARCH,     0 /* architecture 0 means i386 */
.set LEN,      (header_end - header_start) /* length of the header */
.set CHECKSUM, 0x100000000 - (MAGIC + ARCH + LEN) /* checksum of above, to prove we are multiboot2 */

/* 
Declare a multiboot2 header that marks the program as a kernel. These are magic
values that are documented in the multiboot2 standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot2
header_start:
.align 4
.long MAGIC
.long ARCH
.long LEN
.long CHECKSUM
.long 0 # End tag
.long 8 # End tag
header_end:

.section .bss
.global stack_top
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:


# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.global boot_page_directory
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
boot_page_table2:
	.skip 4096

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:





	movl $(boot_page_table1 - 0xC0000000), %edi

	movl $0, %esi
	movl $1024, %ecx


# Add C00 to the start of physical addresses to make them virtual
1:
    movl %esi, %edx
    orl $0x003, %edx      # Present + Writable
    movl %edx, (%edi)

    addl $4096, %esi      # Next physical page
    addl $4, %edi         # Next page table entry
    loop 1b               # Repeat 1024 times

	movl $(boot_page_table2 - 0xC0000000), %edi
	movl $1024, %ecx

2:
	movl %esi, %edx
	orl $0x003, %edx

	addl $4096, %esi
	addl $4, %edi
	loop 2b


3:
	


	# Overwrite 0:1024 (Table 0, page 1024)
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	# Map the first 4MB
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4 

	# Map the second 4MB
	movl $(boot_page_table2 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 4
	movl $(boot_page_table2 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 769 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3



	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0



	# Jump to higher half with an absolute jump. 
	lea higher_half, %ecx
	jmp *%ecx


/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start



.section .text
higher_half:

	

	# Set up the stack
	mov $stack_top, %esp
	

	cli


	push %ebx
	push %eax
	call multiboot_shit


	# Start setting up everything
	call boot_main


	mov $0x0, %eax
	cpuid
	push %ecx
	push %edx
	push %ebx



	call kernel_startup


	cli
1:	hlt
	jmp 1b





