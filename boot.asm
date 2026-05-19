; boot.asm - Wind OS / Sky Core OS Bootloader Giriş Noktası
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_GRAPHICS      equ 1 << 2  ; Grafik modu için gerekli flag
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_GRAPHICS
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM
    ; VBE Modu İsteği
    dd 0, 0, 0, 0, 0
    dd 0          ; Mode type (0 = Linear)
    dd 1024       ; Width
    dd 768        ; Height
    dd 32         ; Depth

section .text
global _start
extern kernel_main

_start:
    push eax      ; Magic number'ı kernel'a yolla
    push ebx      ; Multiboot header adresini yolla
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
