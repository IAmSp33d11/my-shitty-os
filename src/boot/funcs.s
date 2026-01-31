.section .data
gdtr:
     .word 0 # For limit storage
     .long 0 # For base storage

.section .text
.global setGdt
setGdt:
   movw %ax, 4(%esp)
   movw %ax, gdtr
   movl %eax, 8(%esp)
   movl gdtr + 2, %eax
   lgdt [gdtr]

   jmp reloadSegments

reloadSegments:
   # Reload CS register containing code selector:
   lcall $0x08, $reload_CS
   ret


reload_CS:
   # Reload data segment registers:
   
   movw 0x10, %ax # 0x10 is data segment
   movw %ax, %ds 
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs
   movw %ax, %ss
   
   lret