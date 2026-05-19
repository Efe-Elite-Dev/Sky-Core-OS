ENTRY(_start)

SECTIONS {
    /* GRUB kernel'ları genelde 1MB (0x100000) noktasına yükler */
    . = 0x100000; 

    .text : {
        *(.multiboot) /* Multiboot header en başta olmak ZORUNDA */
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss) }
}
