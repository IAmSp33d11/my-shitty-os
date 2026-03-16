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