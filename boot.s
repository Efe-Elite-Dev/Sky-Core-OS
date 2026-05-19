# boot.s - Sky-Core OS Multiboot Header & Entry Point (GNU Assembler Syntax)
.intel_syntax noprefix

# Multiboot Sabitleri
.set MULTIBOOT_ALIGN,     1 << 0
.set MULTIBOOT_MEMINFO,   1 << 1
.set MULTIBOOT_GRAPHICS,  1 << 2  # GRUB'a grafik ekranı zorunlu kıl diyoruz
.set MULTIBOOT_FLAGS,     MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_GRAPHICS
.set MULTIBOOT_MAGIC,     0x1BADB002
.set MULTIBOOT_CHECKSUM,  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# Multiboot Başlığı (GRUB Ayarları)
.section .multiboot
.align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    # Ekran Çözünürlük ve LFB İstekleri
    .long 0
    .long 800   # Genişlik (Width)
    .long 600   # Yükseklik (Height)
    .long 32    # Renk Derinliği (BPP)

# Giriş Noktası ve Çekirdek Bağlantısı
.section .text
.global _start
.extern kernel_main

_start:
    cli
    mov esp, OFFSET stack_top   # Stack işaretçisini güvenli alana taşı
    
    # GRUB'ın donanımdan topladığı kritik verileri C çekirdeğine gönderiyoruz
    push ebx                    # Multiboot Bilgi Yapısı adresi (MBI) -> kernel.c içerisindeki mbi[]
    push eax                    # Boot Sihirli Numarası -> kernel.c içerisindeki magic
    
    call kernel_main            # C dilinde yazdığımız ana çekirdeği tetikle
    
.halt:
    hlt                         # Eğer çekirdekten çıkış olursa işlemciyi güvenli moda al
    jmp .halt

# Yığın (Stack) Bellek Alanı
.section .bss
.align 16
stack_bottom:
    .skip 16384                 # 16KB Güvenli Stack Alanı ayır
stack_top:
