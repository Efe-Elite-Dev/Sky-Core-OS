#!/bin/bash
set -e # Herhangi bir hata oluşursa betiği durdur

echo "======================================================================"
echo "🚀 SKY CORE OS / WIND OS MEGA BUILD ENGINE"
echo "======================================================================"

# 1. Temizlik
echo "[-] Eski derleme kalıntıları temizleniyor..."
rm -rf *.o os_image.iso iso_root

# 2. Assembly Bootloader Derlemesi
echo "[1] Çekirdek önyükleme mekanizması derleniyor (boot.asm)..."
nasm -f elf32 boot.asm -o boot.o

# 3. C Alt Sistemlerinin ve Kernelin Derlenmesi
echo "[2] Çekirdek ve tüm alt sistemler derleniyor..."

# Temel modüller
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c setup.c -o setup.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c setup_ui.c -o setup_ui.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector

# Yeni eklenen donanım ve UI alt sistemleri
gcc -m32 -c gui.c -o gui.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c screen.c -o screen.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c mouse.c -o mouse.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c keyboard.c -o keyboard.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c idt.c -o idt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector

# İleri düzey subsistemler
gcc -m32 -c wind_subsystem.c -o wind_subsystem.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c exe_subsystem.c -o exe_subsystem.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c ai_subsystem.c -o ai_subsystem.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
gcc -m32 -c deb_subsystem.c -o deb_subsystem.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector

# 4. Bağlama (Linker) Aşaması
echo "[3] Tüm nesne dosyaları linker.ld şablonuna göre birleştiriliyor..."
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o setup.o setup_ui.o gui.o screen.o mouse.o keyboard.o idt.o wind_subsystem.o exe_subsystem.o ai_subsystem.o deb_subsystem.o

# 5. Grub ISO Oluşturma
echo "[4] Boot edilebilir ISO imajı paketleniyor..."
mkdir -p iso_root/boot/grub
cp kernel.bin iso_root/boot/kernel.bin

# Basit bir grub.cfg oluşturma (Eğer yoksa)
cat << EOF > iso_root/boot/grub/grub.cfg
set timeout=0
set default=0

menuentry "Sky Core OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o os_image.iso iso_root
echo "[+] BAŞARILI: os_image.iso hazır aga!"
