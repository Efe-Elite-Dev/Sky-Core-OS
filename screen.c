#include "screen.h"

// Eğer linker başka bir dosyada bu değişkenleri bulamazsa, 
// derleme hatasını önlemek için buraya fallback (yedek) tanımlarını gömüyoruz.
// Mode 13h için varsayılan VRAM adresi 0xA0000, pitch (satır genişliği) ise 320 bayttır.
uint8_t* vbe_vram = (uint8_t*)0xA0000;
uint32_t vbe_pitch = 320;

// Ekran yırtılmalarını (tearing) önlemek için arka tampon (Backbuffer)
static uint8_t screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void screen_init(void) {
    // Tampon belleği temizle
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screen_buffer[i] = 0;
    }
}

void screen_put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        screen_buffer[y * SCREEN_WIDTH + x] = color;
    }
}

// Bağlantı hatasını bitiren ve arka tamponu gerçek VRAM'e basan fonksiyon
void screen_flush(void) {
    if (!vbe_vram) return;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // vbe_pitch kullanarak her ekran kartı mimarisine tam uyum sağlıyoruz
            vbe_vram[y * vbe_pitch + x] = screen_buffer[y * SCREEN_WIDTH + x];
        }
    }
}
