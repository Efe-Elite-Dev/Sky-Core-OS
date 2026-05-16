#!/bin/bash

echo "==> Wind OS Evrensel Hibrit UEFI/BIOS ISO Üretimi Başlatıldı..."

# 1. Temizlik
rm -rf isodir windos_boot_layer
rm -f *.o kernel.bin windos.iso

# 2. Klasör Yapısı (Hem Legacy hem UEFI için)
mkdir -p windos_boot_layer/boot/grub
mkdir -p windos_boot_layer/EFI/BOOT

# 3. Önyükleyiciyi derle
nasm -f elf32 boot.asm -o boot.o

# 4. ÇEKİRDEK VE DERİN AI ODALARI
echo "==> Çekirdek modülleri derleniyor..."
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c kernel.c -o kernel.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c gui.c -o gui.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c exe_subsystem.c -o exe_subsystem.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c ai_subsystem.c -o ai_subsystem.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c mouse.c -o mouse.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c keyboard.c -o keyboard.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c wind_subsystem.c -o wind_subsystem.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c screen.c -o screen.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c idt.c -o idt.o || true
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c deb_subsystem.c -o deb_subsystem.o || true

# 5. Linker
gcc -m32 -T linker.ld -nostdlib -no-pie -Wl,-z,noexecstack -o kernel.bin \
    boot.o kernel.o gui.o exe_subsystem.o ai_subsystem.o \
    mouse.o keyboard.o wind_subsystem.o screen.o idt.o deb_subsystem.o || true

# 6. Çekirdek kopyalarını yerleştir
cp kernel.bin windos_boot_layer/boot/kernel.bin || true

# 7. GRUB Menüsü
cat << 'EOF' > windos_boot_layer/boot/grub/grub.cfg
set timeout=0
set default=0
menuentry "Wind OS - Full AI Core" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# 8. ISO'YU EVRENSEL METOTLA BAS
echo "==> grub-mkrescue ile hibrit ISO mühürleniyor..."
grub-mkrescue -o windos.iso windos_boot_layer

echo "==> İşlem bitti!"
