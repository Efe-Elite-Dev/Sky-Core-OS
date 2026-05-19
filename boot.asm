section .multiboot
align 4
    MULTIBOOT_MAGIC    equ 0x1BADB002
    MULTIBOOT_FLAGS    equ 0x00000007
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; Padding (Gereksiz ama GRUB offset hizalaması için zorunlu alanlar)
    dd 0 ; header_addr
    dd 0 ; load_addr
    dd 0 ; load_end_addr
    dd 0 ; bss_end_addr
    dd 0 ; entry_addr

    ; Grafik Modu İstekleri
    dd 0    ; mode_type (0 = VBE)
    dd 1024 ; genişlik
    dd 768  ; yükseklik
    dd 32   ; renk derinliği

; --- KESİN ÇÖZÜM: STACK ALANI TANIMLAMA ---
section .bss
align 16
stack_bottom:
    resb 16384 ; Kernel için 16 KB güvenli yığın alanı
stack_top:

; --- KERNEL GİRİŞ NOKTASI ---
section .text
global _start
extern kernel_main ; kernel.c içindeki ana fonksiyonunun adı neyse (main veya kernel_main)

_start:
    cli ; 1. Kural: Bütün donanım kesmelerini kapat!

    mov esp, stack_top ; 2. Kural: Stack Pointer'ı (esp) buraya eşitle!

    ; GRUB'dan gelen Multiboot parametrelerini C koduna aktarmak istersen:
    push ebx ; Multiboot info yapısı
    push eax ; Multiboot magic numarası

    ; Artık C dünyasına güvenle geçebiliriz
    call kernel_main

.hang:
    hlt ; Eğer C kodundan çıkılırsa işlemciyi uykuya al
    jmp .hang
