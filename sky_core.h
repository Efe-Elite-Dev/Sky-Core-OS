#ifndef SKY_CORE_H
#define SKY_CORE_H

#include <stdint.h>

// Global Değişkenler (Diğer dosyalar da buraya bakacak)
extern uint32_t* FRAMEBUFFER;
extern int SCREEN_W;
extern int SCREEN_H;

// Grafik Fonksiyonları (Prototype)
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color);
void render_ui(void);

// Kernel Giriş Noktası
extern void kernel_main(void* mboot_ptr, uint32_t magic);

#endif
