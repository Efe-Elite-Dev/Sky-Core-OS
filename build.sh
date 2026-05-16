#!/bin/bash
set -e

echo "==> Wind OS Özellikleri Korunarak Yeniden Paketleniyor..."

# 1. Eski kalıntıları temizle
rm -rf isodir
rm -f *.o kernel.bin windos.iso
mkdir -p isodir/boot/grub

# 2. Önyükleyiciyi Derle
nasm -f elf32 boot.asm -o boot.o

# 3. TÜM YAPAY ZEKA VE SÜRÜCÜ ODALARI (HİÇBİR ÖZELLİK SİLİNMEDİ)
echo "==> Sürücü odaları ve AI motorları derleniyor..."
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c gui.c -o gui.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c exe_subsystem.c -o exe_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c ai_subsystem.c -o ai_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c mouse.c -o mouse.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c keyboard.c -o keyboard.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c wind_subsystem.c -o wind_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c screen.c -o screen.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c idt.c -o idt.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c deb_subsystem.c -o deb_subsystem.o

# 4. Hizalamalı Bağlama (Linker)
gcc -m32 -T linker.ld -nostdlib -no-pie -o kernel.bin \
    boot.o kernel.o gui.o exe_subsystem.o ai_subsystem.o \
    mouse.o keyboard.o wind_subsystem.o screen.o idt.o deb_subsystem.o

# 5. Dosyaları Yerleştir
cp kernel.bin isodir/boot/kernel.bin
cp grub.cfg isodir/boot/grub/grub.cfg

# 6. %100 GARANTİLİ GRUB ISO GENERATOR (Kırılmaz Standart)
# Bu komut, grub'ın kendi iç mekanizmasını kullanarak sisteme en uygun hibrit imajı basar
grub-file --is-x86-multiboot isodir/boot/kernel.bin

echo "==> GRUB Rescue imajı inşa ediliyor..."
grub-mkrescue -d /usr/lib/grub/i386-pc -o windos.iso isodir

echo "==> Tüm yapay zeka özellikleri korundu, ISO taş gibi hazır aga!"
