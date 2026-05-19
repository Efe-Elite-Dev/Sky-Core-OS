section .multiboot
align 4
    MULTIBOOT_MAGIC    equ 0x1BADB002
    MULTIBOOT_FLAGS    equ 0x00000007 ; Grafik modu AKTİF edildi (0x03'tü, 0x07 yaptık)
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; Padding (Zorunlu)
    dd 0 
    dd 0 
    dd 0 
    dd 0 
    dd 0 

    ; Grafik Modu Çözünürlük Ayarları
    dd 0    ; 0 = VBE Linear Framebuffer
    dd 1024 ; Genişlik
    dd 768  ; Yükseklik
    dd 32   ; 32 bit renk derinliği

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB Stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli 
    mov esp, stack_top 

    ; GRUB'dan gelen bilgileri kernel_main(mboot_ptr, magic) içine gönder
    push ebx ; mboot_ptr (Pointer to multiboot info)
    push eax ; magic (0x2BADB002)

    call kernel_main

.hang:
    hlt 
    jmp .hang
