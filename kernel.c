void kernel_main(void) {
    // VGA Metin Modu belleği 0xB8000 adresinden başlar.
    volatile unsigned short* vga_buffer = (unsigned short*)0xB8000;
    
    // 1. Ekranı Temizle (Bütün ekranı boşluk karakteri ve siyah arkaplan yap)
    for(int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (unsigned short)(' ' | (0x07 << 8));
    }

    // 2. Ekrana Yazılacak Mesajlar
    const char* line1 = "Wind OS Saf Kernel Modu Basariyla Yuklendi.";
    const char* line2 = "root@windos:/# _";
    
    // 3. Birinci Satırı Yazdır (0x0F = Parlak Beyaz Yazı, Siyah Arkaplan)
    int index = 0;
    while(line1[index] != '\0') {
        vga_buffer[index] = (unsigned short)(line1[index] | (0x0F << 8));
        index++;
    }

    // 4. İkinci Satırı Yazdır (80 karakter sonrasına, yani bir alt satıra geçiyoruz)
    int index2 = 0;
    while(line2[index2] != '\0') {
        vga_buffer[80 + index2] = (unsigned short)(line2[index2] | (0x0A << 8)); // 0x0A = Yeşil Terminal Rengi
        index2++;
    }
    
    // 5. Kernel Döngüsü (İşlemciyi uyutarak beklet)
    while(1) {
        __asm__ volatile("hlt");
    }
}
