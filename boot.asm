[BITS 32]

MULTIBOOT_ALIGN     equ 1 << 0
MULTIBOOT_MEMINFO   equ 1 << 1
MULTIBOOT_GRAPHICS  equ 1 << 2

MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_GRAPHICS
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4

dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

dd 0
dd 800
dd 600
dd 32

section .text
global _start
extern kernel_main

_start:
    cli

    mov esp, stack_top

    ; GRUB parametreleri
    push ebx
    push eax

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16

stack_bottom:
    resb 16384

stack_top:
