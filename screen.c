#include "screen.h"

// Küresel ekran değişkenleri
uint8_t* vbe_vram = (uint8_t*)0xA0000;
uint32_t vbe_pitch = 320;

static uint8_t screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// TÜM SİSTEMLERİN ARADIĞI KÜRESEL SAF PİKSEL SÜRÜCÜSÜ
void draw_pixel_pure(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        vbe_vram[y * vbe_pitch + x] = color;
    }
}

void screen_init(void) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screen_buffer[i] = 0;
    }
}

void screen_put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        screen_buffer[y * SCREEN_WIDTH + x] = color;
    }
}

void screen_flush(void) {
    if (!vbe_vram) return;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            vbe_vram[y * vbe_pitch + x] = screen_buffer[y * SCREEN_WIDTH + x];
        }
    }
}
