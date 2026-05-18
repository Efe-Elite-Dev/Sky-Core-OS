#!/bin/bash
set -e 

echo "======================================================================"
echo "🚀 SKY CORE OS / WIND OS - SAF VE BAĞIMSIZ ULTRA DERLEME MOTORU"
echo "======================================================================"

# 1. Eski Kalıntıları Temizle
echo "[-] Eski nesne dosyaları imha ediliyor..."
rm -rf *.o kernel.bin os_image.iso iso_root

# 2. Assembly Önyükleyici Derlemesi
echo "[1] boot.asm derleniyor..."
nasm -f elf32 boot.asm -o boot.o

# 3. Tüm C Alt Sistemlerini Tek Tek Derle
echo "[2] Çekirdek ve tüm alt modüller derleniyor..."
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c setup.c -o setup.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c setup_ui.c -o setup_ui.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c gui.c -o gui.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c screen.c -o screen.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c mouse.c -o mouse.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c keyboard.c -o keyboard.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c idt.c -o idt.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c wind_subsystem.c -o wind_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c exe_subsystem.c -o exe_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c ai_subsystem.c -o ai_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c deb_subsystem.c -o deb_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector

# 4. Linker ile Tüm Yapıyı Birleştir
echo "[3] Nesne dosyaları tek parça kernel.bin olarak mühürleniyor..."
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o setup.o setup_ui.o gui.o screen.o mouse.o keyboard.o idt.o wind_subsystem.o exe_subsystem.o ai_subsystem.o deb_subsystem.o

# 5. Bağımsız ISO Dizin Yapısı Kurulumu
echo "[4] ISO kök dizini inşa ediliyor..."
mkdir -p iso_root/boot/grub
cp kernel.bin iso_root/boot/kernel.bin

# Saf GRUB Konfigürasyonu
cat << EOF > iso_root/boot/grub/grub.cfg
set timeout=0
set default=0
menuentry "Sky Core OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# 6. Ultra Güvenli Paketleme Motoru
echo "[5] Xorriso ham modda ISO basıyor..."
# Sistemde eltorito imajı arama zorunluluğunu kaldırıyoruz, bulamazsa direkt düz ISO yazar
xorriso -as mkisofs -R -J -V "SKYCOREOS" -o os_image.iso iso_root

echo "======================================================================"
echo "✅ İŞLEM TAMAM: os_image.iso sıfırdan ve pürüzsüzce üretildi!"
echo "======================================================================"
