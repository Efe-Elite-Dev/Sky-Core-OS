#include <stdint.h>
#include <stddef.h>
#include "sky_core.h"

// Değişkenleri burada tanımlıyoruz (extern değil)
uint32_t* FRAMEBUFFER = (uint32_t*)0xFD000000;
int SCREEN_W = 1024;
int SCREEN_H = 768;

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        FRAMEBUFFER[y * SCREEN_W + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            draw_pixel(j, i, color);
        }
    }
}

void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            // Köşe yuvarlama mantığı
            if ((i < y + radius || i > y + h - radius - 1) && 
                (j < x + radius || j > x + w - radius - 1)) continue; 
            draw_pixel(j, i, color);
        }
    }
}

void render_ui(void) {
    // 1. Arka Plan (Gradient)
    for (int i = 0; i < SCREEN_H; i++) {
        uint32_t color = (0xFF << 24) | ((240 - i/10) << 16) | ((244 - i/15) << 8) | (248 - i/20);
        for (int j = 0; j < SCREEN_W; j++) draw_pixel(j, i, color);
    }
    // 2. Ana Kart (Modern OOBE)
    draw_rounded_rect(200, 150, 624, 468, 20, 0xFFFFFFFF);
    // 3. Buton
    draw_rounded_rect(250, 450, 150, 40, 10, 0xFF2575FC);
}

void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)magic;
    (void)mboot_ptr;
    render_ui();
    while(1) { __asm__ volatile("hlt"); }
}
