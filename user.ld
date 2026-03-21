ENTRY(_start)
. = 0x400000;

SECTIONS {
    .text.start : { *(.text._start) }
    .text : { *(.text) }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    .bss : { *(.bss) }
}