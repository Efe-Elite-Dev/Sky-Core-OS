; ==============================================================================
; WIND OS / SKY CORE OS v1.5 - Bootloader Arayüzü (boot.asm)
; Mimari   : x86 (IA-32) - NASM Syntax
; Özellik  : Multiboot 1 Standartlarına Uygun Grafik Modu Açıcı
; Geliştirici: Feyzula Efe Tuna
; ==============================================================================

; Multiboot Tanımlamaları
MBALIGN     equ  1 << 0            ; Sayfa sınırlarına göre hizala
MEMINFO     equ  1 << 1            ; Bellek haritasını kernel'a bildir
VIDEO_MODE  equ  1 << 2            ; Grafik (VBE) modunu zorunlu kıl
FLAGS       equ  MBALIGN | MEMINFO | VIDEO_MODE
MAGIC       equ  0x1BADB002        ; Önyükleyici sihirli numarası
CHECKSUM    equ -(MAGIC + FLAGS)   ; Hata kontrol toplamı

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
    ; Grafik Modu İçin Ek Multiboot Parametreleri (Aga burası çok kritik!)
    dd 0    ; mode_type: 0 = Doğrusal Grafik Modu (Linear Framebuffer)
    dd 1024 ; width: Genişlik (Piksel)
    dd 768  ; height: Yükseklik (Piksel)
    dd 32   ; depth: Renk Derinliği (32-bit ARGB/XRGB)

section .bss
align 16
stack_bottom:
    resb 16384                     ; Kernel için 16 KiB güvenli Stack alanı
stack_top:

section .text
global _start
extern kernel_main                 ; kernel.c içerisindeki ana fonksiyon

_start:
    ; 1. Stack Pointer'ı güvenli alana taşı
    mov esp, stack_top

    ; 2. Kesmeleri (Interrupts) geçici olarak kapat
    cli

    ; 3. GRUB'ın getirdiği Multiboot parametrelerini Stack'e bas
    ; kernel_main(void *mboot_ptr, uint32_t magic) sırasına göre push'luyoruz
    push eax                       ; Magic numara (0x2BADB002 gelmeli)
    push ebx                       ; mboot_ptr (Ekran kartı bellek adresini barındıran yapı)

    ; 4. Kernel dünyasına büyük adım!
    call kernel_main

.hang:
    cli
    hlt                            ; Kernel kapanırsa işlemciyi sonsuz uykuya al
    jmp .hang
