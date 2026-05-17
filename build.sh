#!/bin/bash
# Hata oluşursa betiğin çalışmasını hemen durdur (Fail Fast)
set -e

echo "=== Wind OS Derleme Motoru Başlatıldı ==="

# Eski kalıntıları temizle
rm -rf iso kernel.bin *.o

# 1. Assembly Dosyasını Derle
echo "[1/4] Assembly derleniyor..."
nasm -f elf32 boot.asm -o boot.o

# 2. C Dosyalarını Derle (setup.c dahil)
echo "[2/4] C kaynak kodları derleniyor..."
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
gcc -m32 -c setup.c -o setup.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# 3. Bağlama (Link) Aşaması - CRITICAL SIRA!
echo "[3/4] Nesne dosyaları bağlanıyor (Linker)..."
# boot.o kesinlikle en başta olmalı (Multiboot imzası için)
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o setup.o

# Çekirdeğin başarıyla üretildiğini doğrula
if [ ! -f "kernel.bin" ]; then
    echo "HATA: kernel.bin linker tarafından üretilemedi!"
    exit 1
fi

# 4. ISO Yapısını Kurgula
echo "[4/4] ISO hiyerarşisi hazırlanıyor..."
mkdir -p iso/boot/grub

# Çekirdeği tam olarak GRUB'ın aradığı yere kopyala
cp kernel.bin iso/boot/kernel.bin

# grub.cfg dosyasını garantiye al
cat << EOF > iso/boot/grub/grub.cfg
set timeout=0
set default=0

menuentry "Wind OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# ISO Dosyasını Mühürle
grub-mkrescue -o windos.iso iso

echo "=== BAŞARILI: windos.iso başarıyla üretildi! ==="
