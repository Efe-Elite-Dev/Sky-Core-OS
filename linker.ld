ENTRY(_start)

SECTIONS
{
    /* Çekirdek tam 1MB (0x100000) adresine oturacak */
    . = 0x100000;

    .text BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot)) /* Derleyicinin başlığı silmesini veya kaydırmasını engeller */
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) { *(.rodata*) }
    .data BLOCK(4K)   : ALIGN(4K) { *(.data)   }
    .bss BLOCK(4K)    : ALIGN(4K) { *(.bss)    }
}
