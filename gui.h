#ifndef GUI_H
#define GUI_H
#include "globals.h"

// Çekirdek ve Başlangıç Fonksiyonları
void init_vga(void);
void clear_screen(uint32_t color);

// Grafik Çizim Temelleri
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_text(const char* text, int x, int y, uint32_t color);

// Pencere ve Bileşen Çiziciler
void draw_window_pure(int x, int y, int w, int h, const char* title);
void draw_button(const char* label, int x, int y, int w, int h, bool focused);

// Arayüz Ekranları (2. Görsel Arayüz Yapısı)
void draw_setup_welcome(void);
void draw_setup_location(void);
void draw_setup_completing(void);
void draw_main_desktop(void);
void draw_ai_subsystem_hud(void);

#endif
