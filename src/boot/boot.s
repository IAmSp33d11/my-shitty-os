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

/*
The multiboot2 standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.global stack_top
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/



	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp

	# So we apparently need to save multiboot2 stuff so...
	mov %ebx, [0xCAFE]
	mov %eax, [0xCAFEE]
	

	cli
	# Prepare the arguments
	mov [0xCAFE], %ebx
	mov [0xCAFEE], %eax

	push %ebx
	push %eax
	call multiboot_shit

	# Start setting up everything
	call boot_main

	mov $0x0, %eax
	cpuid
	mov %ecx, [0xCCCC8]
	mov %edx, [0xCCCC4]
	mov %ebx, [0xCCCC0]




	call kernel_main


	cli
1:	hlt
	jmp 1b





/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start