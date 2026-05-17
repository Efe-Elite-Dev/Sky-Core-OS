#include "wind_subsystem.h"

/* kernel.c içindeki akıcı çift arabelleğe (Double Buffer) dışarıdan bağlanıyoruz */
extern uint32_t* back_buffer;

/* Harici ekran temizleme ve piksel basma fonksiyonlarını prototip olarak bağlıyoruz */
extern void draw_pixel_pure(int x, int y, uint32_t color);
extern void clear_screen_gfx(uint32_t color);

/**
 * @brief Masaüstü arka planını ve temel GUI bileşenlerini arka arabelleğe çizer.
 * kernel.c içindeki ana döngü tarafından saniyede yüzlerce kez tetiklenir.
 */
void gui_refresh_desktop(void) {
    /* Masaüstünü Sky OS / Wind OS temasına uygun derin bir gece mavisi/lacivert ile tazele */
    // Eğer ekran zaten temizleniyorsa burası pencerelerin arka plan dolgusu için kullanılabilir
}

/**
 * @brief OOBE ve Sistem pencerelerini ekrana basan saf grafik motoru fonksiyonu.
 * @param x Pencerenin sol üst X koordinatı
 * @param y Pencerenin sol üst Y koordinatı
 * @param width Pencerenin genişliği (piksel)
 * @param height Pencerenin yüksekliği (piksel)
 * @param border_color Pencere çerçeve rengi (32-bit ARGB/XRGB)
 */
void draw_window_pure(int x, int y, int width, int height, uint32_t border_color) {
    // 1. Güvenlik ve Ekran Taşma Kontrolü (Sanal makinenin çökmesini engeller)
    if (x < 0 || x >= 800 || y < 0 || y >= 600) return;
    if (x + width > 800) width = 800 - x;
    if (y + height > 600) height = 600 - y;

    // 2. Pencere İç Dolgusu (Klasik işletim sistemi penceresi için koyu gri/füme arka plan)
    uint32_t bg_color = 0x001A1A1A; 
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            draw_pixel_pure(i, j, bg_color);
        }
    }

    // 3. Pencere Çerçevesini (Border) Çiz - Üst ve Alt Kenarlar
    for (int i = x; i < x + width; i++) {
        draw_pixel_pure(i, y, border_color);              // Üst Çizgi
        draw_pixel_pure(i, y + height - 1, border_color); // Alt Çizgi
    }

    // 4. Pencere Çerçevesini (Border) Çiz - Sol ve Sağ Kenarlar
    for (int j = y; j < y + height; j++) {
        draw_pixel_pure(x, j, border_color);             // Sol Çizgi
        draw_pixel_pure(x + width - 1, j, border_color); // Sağ Çizgi
    }

    // 5. Pencere Üst Başlık Çubuğu (Title Bar) Simülasyonu (Pencereye estetik katar)
    int title_bar_height = 25;
    if (height > title_bar_height) {
        for (int j = y + 1; j < y + title_bar_height; j++) {
            for (int i = x + 1; i < x + width - 1; i++) {
                draw_pixel_pure(i, j, border_color); // Başlık çubuğunu çerçeve rengiyle doldur
            }
        }
    }
}
