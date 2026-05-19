; boot.s - Multiboot Header (Bunu mutlaka ekle, yoksa grafik gelmez)
section .multiboot
align 4
header_start:
    dd 0x1BADB002             ; Magic number
    dd 0x00010003             ; Flags: Align, Meminfo, Graphics
    dd -(0x1BADB002 + 0x00010003) ; Checksum

    ; Grafik modu isteği (1024x768x32)
    dd 0, 0, 0, 0, 0          ; Header addr
    dd 0                      ; Mode type (0 = Linear Graphics)
    dd 1024                   ; Width
    dd 768                    ; Height
    dd 32                     ; Depth
header_end:
