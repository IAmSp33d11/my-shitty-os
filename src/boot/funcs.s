.section .data
gdtr:
     .word 47 # For limit storage
     .long 0x50000 # For base storage

.section .text
.global setGdt
setGdt:
   lgdt  gdtr

   
   # Reload CS register containing code selector:
   ljmp $0x08, $reload_CS

   


reload_CS:
   # Reload data segment registers:

   movw $0x10, %ax # 0x10 is data segment
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs
   movw %ax, %ss
   
   ret

.extern saved_kernel_esp
.extern saved_kernel_ebp
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