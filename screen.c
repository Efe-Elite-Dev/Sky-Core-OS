#include "wind_subsystem.h"

/* kernel.c içindeki akıcı çift arabellek dizisine dışarıdan bağlanıyoruz */
extern uint32_t* back_buffer;

void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void init_graph_mode(void) {
    // VBE grafik modu GRUB tarafından boot.asm üzerinden otomatik açılıyor.
}

/* İşte o meşhur fonksiyonun modern, zırhlı hali! */
void draw_pixel_pure(int x, int y, uint32_t color) {
    // Sınır kontrolü: Ekrandan taşmaları engelle, sanal makineyi çökertme!
    if (x < 0 || x >= 800 || y < 0 || y >= 600) return;
    
    // Pikseli doğrudan 5MB/32MB güvenli arabelleğe yazıyoruz
    back_buffer[y * 800 + x] = color;
}

void clear_screen_gfx(uint32_t color) {
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            draw_pixel_pure(x, y, color);
        }
    }
}
