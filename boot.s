section .multiboot
align 4
    MAGIC equ 0x1BADB002
    FLAGS equ 0x03 ; Sadece sayfa hizalama ve hafıza bilgisi (Grafik KAPALI)
    CHECKSUM equ -(MAGIC + FLAGS)

    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; Kernel için 16 KB temiz yığın alanı
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli ; Kesmeleri kapat
    mov esp, stack_top ; Stack'i ayarla

    call kernel_main ; C koduna geç

.hang:
    hlt ; İşlemciyi uyut
    jmp .hang
