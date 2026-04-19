.section .data
gdtr:
     .word 47 # For limit storage
     .long 0xC0050000 # For base storage

.section .text
.global setGdt
setGdt:
   lgdt  gdtr
   
   
   
   
   # Reload CS register containing code selector:
   ljmp $0x08, $reload_CS

   


reload_CS:
   # Cause we've been in the higher_half for a while, unmap the identity mapped areas
	movl $0, boot_page_directory + 0
	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3


   # Reload data segment registers:

   movw $0x10, %ax # 0x10 is data segment
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs
   movw %ax, %ss
   
   ret

.extern saved_kernel_esp
.extern boot_page_directory
.global kernel_return
kernel_return:
   ljmp $0x08, $kernel_reload_CS

kernel_reload_CS:
   movw $0x10, %ax
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs
   movw %ax, %ss
   # We gotta pretend we are exiting from kernel_main()
   # that way we can continue the kernel code.
   mov saved_kernel_esp, %esp
   pushf
   pop %eax
   or $0x200, %eax     # enable interrupts
   push %eax           # EFLAGS
   push $0x08          # CS kernel
   push $kernel_main   # EIP
   iret

// void jump_usermode(uint32_t address, uint32_t stack_address)
.global jump_usermode
jump_usermode:
   mov 4(%esp), %ebx # get the address we jump to
   mov 8(%esp), %ecx
   movw $0x23, %ax
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs
   push $0x23
   push %ecx
   pushf
   pop %eax
   or $0x200, %eax
   push %eax
   push $0x1B
   push %ebx
   iret