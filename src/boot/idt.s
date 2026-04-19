bits 32

%macro isr_err_stub 1
isr_stub_%+%1:
    push %1
    call exception_handler
    add esp, 4
    iret 
%endmacro
; if writing for 64-bit, use iretq instead
%macro isr_no_err_stub 1
isr_stub_%+%1:
    push %1
    call exception_handler
    add esp, 4
    iret
%endmacro

%macro divide_error_stub 1
isr_stub_%+%1:
    pushad
    cld
    call divide_error
    popad
    iret
%endmacro

%macro keyboard_handler_stub 1
isr_stub_%+%1:
    pushad
    cld
    call keyboard_handler
    popad
    iret
%endmacro

%macro pit_handler_stub 1
isr_stub_%+%1:
    pushad
    cld
    call pit_handler
    popad
    iret
%endmacro

%macro irq_handler_stub 1
isr_stub_%+%1:
    pushad
    cld
    call irq_handler
    popad
    iret
%endmacro

%macro syscall_stub 1
isr_stub_%+%1:
    cld
    push edx
    push ecx
    push ebx
    push eax
    sti ; This is sketchy shit my god.
    ; I don't like this.
    ; Don't do this
    call syscall_handler
    add esp, 16
    iret
%endmacro

%macro gpf_stub 1
isr_stub_%+%1:
    call gpf_handler
    add esp, 4
    iret
%endmacro

%macro pf_stub 1
isr_stub_%+%1:
    push %1
    call exception_handler
    add esp, 4
    iret
%endmacro

extern exception_handler
extern divide_error
extern keyboard_handler
extern pit_handler
extern irq_handler
extern syscall_handler
extern gpf_handler


divide_error_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
gpf_stub    13
pf_stub   14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
pit_handler_stub 32 ; P.I.T. (0)
keyboard_handler_stub 33 ; 1
irq_handler_stub 34 ; 2
irq_handler_stub 35 ; 3
irq_handler_stub 36 ; 4
irq_handler_stub 37 ; 5
irq_handler_stub 38 ; 6
irq_handler_stub 39 ; 7
irq_handler_stub 40 ; 8
irq_handler_stub 41 ; 9
irq_handler_stub 42 ; 10
irq_handler_stub 43 ; 11
irq_handler_stub 44 ; 12
irq_handler_stub 45 ; 13
irq_handler_stub 46 ; 14
irq_handler_stub 47 ; 15
syscall_stub 48

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    49
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep


