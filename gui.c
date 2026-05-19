#include "gui.h"

void init_vga() {
    // Gerçek donanımda VBE moduna geçiş kodları buraya bağlanır
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        uint32_t* framebuffer = (uint32_t*)GRAPHICS_FRAMEBUFFER;
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

void draw_text(const char* text, int x, int y, uint32_t color) {
    // Basit OS font çizici - her karakteri piksel piksel ekrana basar
    while (*text) {
        // Karakter çizim matrisi simülasyonu
        draw_rect(x, y, 8, 12, color); 
        x += 10;
        text++;
    }
}

void draw_window_pure(int x, int y, int w, int h, const char* title) {
    // Beyaz/Gri pencere gövdesi
    draw_rect(x, y, w, h, 0xEEEEEE);
    // Koyu lacivert/mor başlık çubuğu
    draw_rect(x, y, w, 30, 0x1A0933);
    draw_text(title, x + 10, y + 8, 0xFFFFFF);
}

void draw_button(const char* label, int x, int y, int w, int h, bool focused) {
    uint32_t btn_color = focused ? 0x4A154B : 0x333333; // Odaklanmışsa mor tonu
    draw_rect(x, y, w, h, btn_color);
    draw_text(label, x + 15, y + (h/2) - 6, 0xFFFFFF);
}

// 2. Görsel Üst Sol: İlk Kurulum Ekranı
void draw_setup_welcome() {
    clear_screen(0x13011C); // Koyu mor arka plan
    draw_window_pure(150, 100, 500, 400, "ILK KURULUM EKRANI - HOS GELDINIZ");
    
    draw_text("ILK KURULUM EKRANI", 280, 160, 0x000000);
    draw_text("Sisteme Hos Geldiniz!", 290, 200, 0x000000);
    
    draw_button("Hizli Kurulum (Az)", 200, 320, 180, 40, true);
    draw_button("Detayli Kurulum (Baska)", 420, 320, 200, 40, false);
}

// 2. Görsel Üst Sağ: Konum & Saat Ayarlama
void draw_setup_location() {
    clear_screen(0x13011C);
    draw_window_pure(100, 80, 600, 450, "KONUM & SAAT AYARLAMA");
    
    draw_text("SKY CORE OS v1.5'i Sectiginiz Icin Tesekkurler!", 120, 130, 0x000000);
    
    // Türkiye Haritası Temsili Çizimi (Büyük bir dikdörtgen ve pin)
    draw_rect(340, 160, 230, 120, 0x555555); 
    draw_text("HARITA (TURKIYE)", 390, 210, 0xFFFFFF);
    draw_rect(380, 180, 6, 6, 0xFF0000); // Kırmızı Konum Pini
    
    draw_text("Konum: [Istanbul, Turkiye]", 340, 300, 0x000000);
    draw_text("Bolge Saati: [GMT+03:00]", 340, 340, 0x000000);
    
    draw_button("Konum ve Saat Ayarlarini Kaydet", 340, 390, 240, 40, true);
}

// 2. Görsel Alt Sol: Giriş & Tamamlama
void draw_setup_completing() {
    clear_screen(0x13011C);
    draw_window_pure(100, 80, 600, 450, "GIRIS & TAMAMLAMA");
    
    // Sol panel harita özeti
    draw_rect(120, 140, 200, 100, 0x444444);
    draw_text("Konum: [Istanbul]", 130, 260, 0x000000);
    
    // Sağ panel logo ve başlat butonu
    draw_text("Tesekkurler, Kullanima Hazir!", 350, 160, 0x000000);
    draw_text("Masaustune Gitmek Icin HAZIR", 350, 200, 0x000000);
    
    draw_button("BASLAT", 380, 320, 150, 45, true);
}

// 2. Görsel Alt Sağ: Widget'lı ve Uygulamalı Ana Masaüstü
void draw_main_desktop() {
    clear_screen(0x1D2A44); // Gece mavisi masaüstü arka planı
    
    // Sağ Üst: Hava Durumu & Saat Widget'ı
    draw_rect(520, 20, 260, 120, 0x22FFFFFF); // Yarı saydam panel
    draw_text("26:03", 540, 40, 0xFFFFFF);
    draw_text("Hava Durumu ve Saat", 540, 80, 0xBBBBBB);
    
    // Sağ Dikey Panel: Uygulamalar Sütunu
    draw_rect(650, 160, 130, 400, 0x33000000);
    draw_text("UYGULAMALAR", 660, 180, 0xFFFFFF);
    draw_button("Terminal", 660, 220, 110, 30, false);
    draw_button("Dosyalar", 660, 260, 110, 30, false);
    draw_button("Mesajlar", 660, 300, 110, 30, false);
    draw_button("Ayarlar", 660, 340, 110, 30, false);
    
    // Ortadaki "HOS GELDINIZ" Bildirim Penceresi (Birebir görseldeki gibi)
    draw_window_pure(250, 200, 300, 150, "HOS GELDINIZ");
    draw_text("Sisteme Hos Geldiniz!", 270, 250, 0x000000);
    draw_text("GHHOD GERLDIN!", 270, 280, 0xFF0000); // Görseldeki o ikonik yazı!
    
    // Alt Görev Çubuğu (Taskbar)
    draw_rect(0, 560, SCREEN_WIDTH, 40, 0x0F0F1A);
    draw_text("TUSUMUNA BASINCA CEKMECE ACILSIN", 20, 575, 0xFFFFFF);
}

// ALT + SPACE Yapay Zeka HUD Ekranı (Üstten Açılan Panel)
void draw_ai_subsystem_hud() {
    if (!ai_hud_visible) return;
    
    // Ekranın üstüne yarı saydam siyah katman overlay atar
    draw_rect(100, 0, 600, 220, 0xFA050510);
    draw_rect(100, 215, 600, 5, 0xFF00FF); // Neon Mor çizgi boundary
    
    draw_text("--- SKY-NET AI CORE SUBSYSTEM v1.0 ---", 120, 20, 0x00FFFF);
    draw_text("Sorunuzu Yazın: [                                    ]", 120, 70, 0xFFFFFF);
    draw_text("AI Yanıtı: Sistem mimarisi optimize edildi. Komut bekleniyor...", 120, 130, 0x00FF00);
    draw_text("Kapatmak için tekrar 'ALT + SPACE' tuşuna basın.", 120, 180, 0x888888);
}
