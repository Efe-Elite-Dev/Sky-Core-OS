; boot.asm
MULTIBOOT_ALIGN     equ 1 << 0
MULTIBOOT_MEMINFO   equ 1 << 1
MULTIBOOT_GRAPHICS  equ 1 << 2  ; GRUB'a grafik modu aç diyoruz
MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_GRAPHICS
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    ; Grafik Ekran İstek Ayarları
    dd 0
    dd 800  ; Genişlik
    dd 600  ; Yükseklik
    dd 32   ; Renk Derinliği (BPP)

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    
    ; GRUB'ın bize verdiği sistem ve ekran bilgilerini C çekirdeğine paslıyoruz
    push ebx         ; Multiboot bilgi yapısının adresi
    push eax         ; Sihirli numara (Magic Number)
    
    call kernel_main
    
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384       ; 16KB Güvenli Stack Alanı
stack_top:
