# boot.s - Sky-Core OS Multiboot Header & Entry Point (GNU Assembler Syntax)
.intel_syntax noprefix

# Multiboot Sabitleri
.set MULTIBOOT_ALIGN,     1 << 0
.set MULTIBOOT_MEMINFO,   1 << 1
.set MULTIBOOT_GRAPHICS,  1 << 2  
.set MULTIBOOT_FLAGS,     MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_GRAPHICS
.set MULTIBOOT_MAGIC,     0x1BADB002
.set MULTIBOOT_CHECKSUM,  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# Multiboot Başlığı (GRUB Ayarları)
.section .multiboot
.align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    
    # KRİTİK DÜZELTME: Grafik alanlarının 32. byte offsetine oturması için dolgu alanları
    .long 0   # header_addr
    .long 0   # load_addr
    .long 0   # load_end_addr
    .long 0   # bss_end_addr
    .long 0   # entry_addr
    
    # Ekran Çözünürlük ve LFB İstekleri (Tam olarak doğru offsette!)
    .long 0   # mode_type (0 = Lineer Grafik Modu)
    .long 800   # Genişlik (Width)
    .long 600   # Yükseklik (Height)
    .long 32    # Renk Derinliği (BPP)

# Giriş Noktası ve Çekirdek Bağlantısı
.section .text
.global _start
.extern kernel_main

_start:
    cli
    mov esp, OFFSET stack_top   
    
    push ebx                    
    push eax                    
    
    call kernel_main            
    
.halt:
    hlt                         
    jmp .halt

# Yığın (Stack) Bellek Alanı
.section .bss
.align 16
stack_bottom:
    .skip 16384                 
stack_top:
