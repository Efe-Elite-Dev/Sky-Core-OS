/**
 * ==============================================================================
 * 🌟 SKY CORE OS / WIND OS OPERATING SYSTEM 🌟
 * ==============================================================================
 * [Proje Kodu]: Sky Core OS v1.5 (Vortex Kernel)
 * [Mimari]: x86 Intel/AMD IA-32 Monolitik Çekirdek Standartları
 * [Derleme Hedefi]: 32-Bit Korumalı Mod (Protected Mode) - Multiboot Uyumlu
 * [Geliştirici]: Feyzula Efe Tuna
 * ==============================================================================
 * * ÇEKİRDEK MİMARİSİ VE ÇALIŞMA PRENSİPLERİ (DETAYLI TEKNİK ANALİZ):
 * * 1. Grafiksel Durum Makinesi (UI State Machine):
 * Sistem, bootloader (GRUB) aşamasından kontrolü devraldığı an metin tabanlı VGA
 * modunu tamamen baypas eder. `current_os_state` değişkeni üzerinden sistemin
 * hangi kurulum aşamasında olduğu veya masaüstü moduna geçip geçmediği kontrol edilir.
 * * 2. Doğrudan Bellek Erişimi (Linear Framebuffer - LFB):
 * Sistem, ekran kartının VRAM (Video RAM) bölgesine doğrudan `GRAPHICS_FRAMEBUFFER`
 * işaretçisi üzerinden erişir. `0xFD000000` veya GRUB Multiboot tag'inden gelen
 * LFB adresi kullanılarak, ekrandaki her bir piksel tek tek 32-bit renk derinliğiyle
 * (ARGB: Alpha, Red, Green, Blue) mühürlenir.
 * * 3. Donanımsal Polling Klavye Alt Sistemi:
 * İntel 8042 klavye kontrolcüsü (PS/2) sürekli olarak `0x64` portundan dinlenir.
 * Veri hazır olduğunda (`Status Register Bit 0 = 1`), `0x60` veri portundan donanımsal
 * scancode okunur. Bu scancode, çekirdeğin kendi içindeki durum makinesi fonksiyonlarına
 * ve `setup.c` alt sistemine gönderilir.
 */

#include <stdint.h>
#include <stddef.h>

// ==============================================================================
// 🪐 1. SİSTEM DURUM MAKİNESİ VE GLOBAL SABİTLER (STATE MACHINE CONFIG)
// ==============================================================================

/**
 * @brief Sky Core OS'in kullanıcı arayüzü akışını milimetrik kontrol eden ana enum.
 * Bu enum, sistemin boot anından masaüstüne kadar olan tüm yaşam döngüsünü simüle eder.
 */
typedef enum {
    STATE_WELCOME = 0,    // Arama 1: İlk Kurulum Ekranı - Hoş Geldiniz (Panel & Pusula)
    STATE_LOCATION = 1,   // Arama 2: Konum & Saat Ayarlama (Vektörel Türkiye Haritası)
    STATE_SUMMARY = 2,    // Arama 3: Giriş Bilgileri Kontrolü & Tamamlama (Ay İkonu)
    STATE_DESKTOP = 3     // Arama 4: Fırtına ve Ay Temalı, Canavar Widget'lı Masaüstü!
} OS_UI_STATE;

// Global Çekirdek Durum Değişkeni (Kesmeler altında güvenli çalışma için volatile)
volatile OS_UI_STATE current_os_state = STATE_WELCOME;

// Ekran Çözünürlüğü Tanımlamaları (VBE Standartı)
#define SCREEN_WIDTH         1024
#define SCREEN_HEIGHT        768
#define SCREEN_BPP           32      // Piksel Başına Bit (32-Bit TrueColor)

// Donanımsal Renk Paleti Makroları (ARGB Formatı)
#define COLOR_DEEP_PURPLE    0xFF1A0F2E  // Sky Core OS Ana Fırtına Teması Moru
#define COLOR_DARK_BLUE      0xFF0D0B18  // Derin Uzay Siyahı / Gece Mavisi
#define COLOR_WHITE          0xFFFFFFFF  // Panel Arka Planları Beyazı
#define COLOR_LIGHT_GRAY     0xFFE0E0E0  // Pasif Buton Rengi
#define COLOR_ACTIVE_BLUE    0xFF2575FC  // Odaklanmış Buton / Seçim Mavisi
#define COLOR_MAP_GOLD       0xFFFFD700  // Vektörel Türkiye Haritası Altın Sarısı
#define COLOR_TEXT_DARK      0xFF222222  // Koyu Metin Rengi
#define COLOR_TEXT_LIGHT     0xFFF5F5F5  // Açık Metin Rengi
#define COLOR_WIDGET_BG      0x80251F3D  // %50 Saydam Widget Arka Plan Rengi
#define COLOR_GLOW_CYAN      0xFF00E5FF  // Parlayan Neon Camgöbeği (Sistem Bildirimleri)

// ==============================================================================
// 🖥️ 2. DONANIM VE BELLEK ADRESLERİ KATMANI (HARDWARE & MEMORY MAP)
// ==============================================================================

// VGA Metin Modu Bellek Adresi (Yedek Koruma Katmanı)
uint16_t* const TEXT_VIDEO_MEMORY = (uint16_t*)0xB8000;
int text_x = 0;
int text_y = 0;

// VBE Doğrudan Ekran Kartı Piksel Bellek Adresi (Linear Framebuffer)
// Not: Gerçek donanımlarda bu adres PCI veri yolundan okunur, sanal makineler için varsayılan taban:
uint32_t* const GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;

// Çekirdek Grafik Modu Kilidi (0 = Metin Modu, 1 = Sky Core VBE Grafik Modu)
int is_graphics_mode = 1; 

// Klavye Giriş Tamponu (Buffer) ve İndis Takipçisi
char cmd_buffer[16];
int cmd_idx = 0;

// ==============================================================================
// 🎹 3. KLAVYE SCANCODE TO ASCII ÇEVİRİ MATRİSİ (KEYBOARD MAP)
// ==============================================================================
// Küresel x86 donanımsal klavye scancode değerlerini ASCII karakterlerine eşler.
static const char scancode_ascii_table[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',    0,
   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\', 'z',
   'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',   0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,    0,
  '-',    0,    0,    0,  '+',    0,    0,    0,   0,   0,   0,    0,   0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,    0
};

// ==============================================================================
// ⛓️ 4. DIŞ DÜNYA BAĞLANTILARI VE ALT SİSTEMLER (EXTERNAL SUBSYSTEMS)
// ==============================================================================
// setup.c ve gui.c gibi harici modüllerle haberleşme köprüleri.
extern void setup_init(void);
extern void setup_handle_input(uint8_t scancode);

// ==============================================================================
// 🛠️ 5. DÜŞÜK SEVİYELİ İŞLEMCİ PORT GİRİŞ/ÇIKIŞ FONKSİYONLARI (I/O PORTS)
// ==============================================================================

/**
 * @brief Belirtilen I/O portundan 1 baytlık veri okur (Assembly IN talimatı).
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Belirtilen I/O portuna 1 baytlık veri yazar (Assembly OUT talimatı).
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// ==============================================================================
// 📝 6. METİN MODU VE HATA AYIKLAMA MOTORU (TEXT MODE FALLBACK)
// ==============================================================================

void clear_text_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        TEXT_VIDEO_MEMORY[i] = (0x0F << 8) | ' ';
    }
    text_x = 0;
    text_y = 0;
}

void print_string(const char* str) {
    if (is_graphics_mode) return; // Grafik motoru devredeyse metin yazımını kilitle
    while (*str) {
        if (*str == '\n') {
            text_x = 0;
            text_y++;
        } else {
            TEXT_VIDEO_MEMORY[text_y * 80 + text_x] = (0x0F << 8) | *str;
            text_x++;
            if (text_x >= 80) { text_x = 0; text_y++; }
        }
        str++;
    }
}

// ==============================================================================
// 🎨 7. ÇEKİRDEK İÇİ GRAFİK ÇİZİM MOTORU (LOW-LEVEL GRAPHICS ENGINE)
// ==============================================================================
// Ekran kartına doğrudan piksel mühürleyen saf C grafik kütüphanesi.

/**
 * @brief Belirtilen koordinattaki pikselin rengini değiştirir.
 */
static inline void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        GRAPHICS_FRAMEBUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

/**
 * @brief Ekranın tamamını belirtilen renk ile kaplar.
 */
void fill_screen(uint32_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        GRAPHICS_FRAMEBUFFER[i] = color;
    }
}

/**
 * @brief İçi dolu bir dikdörtgen çizer (Paneller ve butonlar için).
 */
void draw_filled_rectangle(int x, int y, int width, int height, uint32_t color) {
    for (int curr_y = y; curr_y < y + height; curr_y++) {
        for (int curr_x = x; curr_x < x + width; curr_x++) {
            put_pixel(curr_x, curr_y, color);
        }
    }
}

/**
 * @brief Dikdörtgen kenarlığı çizer (Pencerelerin dış çizgileri için).
 */
void draw_rectangle_outline(int x, int y, int width, int height, uint32_t color) {
    for (int curr_x = x; curr_x < x + width; curr_x++) {
        put_pixel(curr_x, y, color);
        put_pixel(curr_x, y + height - 1, color);
    }
    for (int curr_y = y; curr_y < y + height; curr_y++) {
        put_pixel(x, curr_y, color);
        put_pixel(x + width - 1, curr_y, color);
    }
}

/**
 * @brief Dikey gradyan geçişi oluşturur (Fırtınalı gökyüzü efekti için).
 */
void draw_vertical_gradient(uint32_t color_top, uint32_t color_bottom) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // İki renk arasında dikey konuma göre interpolasyon (renk karıştırma) hesabı
        uint8_t r = ((color_top >> 16) & 0xFF) * (SCREEN_HEIGHT - y) / SCREEN_HEIGHT + ((color_bottom >> 16) & 0xFF) * y / SCREEN_HEIGHT;
        uint8_t g = ((color_top >> 8) & 0xFF) * (SCREEN_HEIGHT - y) / SCREEN_HEIGHT + ((color_bottom >> 8) & 0xFF) * y / SCREEN_HEIGHT;
        uint8_t b = (color_top & 0xFF) * (SCREEN_HEIGHT - y) / SCREEN_HEIGHT + (color_bottom & 0xFF) * y / SCREEN_HEIGHT;
        uint32_t mixed_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
        
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            GRAPHICS_FRAMEBUFFER[y * SCREEN_WIDTH + x] = mixed_color;
        }
    }
}

/**
 * @brief Çekirdek içi basit bit eşlemli karakter çizici (8x8 Font Simülatörü).
 */
void draw_char_basic(int x, int y, char c, uint32_t color) {
    // 8x8 boyutunda temsili bir 'S' harfi bit matrisi (Örnek amaçlı basitleştirilmiş font verisi)
    static const uint8_t font_mock[8] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00};
    
    for (int r = 0; r < 8; r++) {
        uint8_t row_byte = (c == 'S' || c == 's') ? font_mock[r] : 0xFF; // Diğer karakterler blok basılsın
        for (int b = 0; b < 8; b++) {
            if (row_byte & (1 << (7 - b))) {
                put_pixel(x + b, y + r, color);
            }
        }
    }
}

/**
 * @brief Ekrana yazı metni basar.
 */
void draw_string_graphics(int x, int y, const char* str, uint32_t color) {
    int curr_x = x;
    while (*str) {
        if (*str == ' ') {
            curr_x += 8;
        } else {
            draw_char_basic(curr_x, y, *str, color);
            curr_x += 9; // Karakterler arası boşluk
        }
        str++;
    }
}

// ==============================================================================
// 🖼️ 8. SİSTEM DURUM RENDER MOTORU (SCREEN DRAWING FUNCTIONS)
// ==============================================================================
// Kullanıcının gönderdiği arayüz fotoğraflarını birebir simüle eden çizim alanları.

/**
 * @brief ARAMA 1: İlk Kurulum Ekranı - Hoş Geldiniz
 * Ekranın ortasında dev bir hoş geldiniz paneli, pusula logosu ve kurulum butonları çizer.
 */
void draw_ui_welcome_screen(void) {
    // 1. Arka planı derin fırtınalı mor gradyan ile mühürle
    draw_vertical_gradient(COLOR_DEEP_PURPLE, COLOR_DARK_BLUE);
    
    // 2. Ortadaki Büyük Beyaz Kurulum Panelini İnşa Et
    int panel_w = 650;
    int panel_h = 380;
    int panel_x = (SCREEN_WIDTH - panel_w) / 2;
    int panel_y = (SCREEN_HEIGHT - panel_h) / 2 - 40;
    draw_filled_rectangle(panel_x, panel_y, panel_w, panel_h, COLOR_WHITE);
    draw_rectangle_outline(panel_x, panel_y, panel_w, panel_h, COLOR_GLOW_CYAN);
    
    // 3. Üst Pencere Başlığı
    draw_string_graphics(panel_x + 30, panel_y + 30, "ILK KURULUM EKRANI - HOS GELDINIZ", COLOR_TEXT_DARK);
    
    // 4. Pusula İkon Alanı (Merkezi Yuvarlak Temsili Alan)
    draw_filled_rectangle(panel_x + panel_w/2 - 40, panel_y + 80, 80, 80, COLOR_DARK_BLUE);
    draw_string_graphics(panel_x + panel_w/2 - 25, panel_y + 115, "PUSULA", COLOR_TEXT_LIGHT);
    
    // 5. Karşılama Metinleri
    draw_string_graphics(panel_x + 120, panel_y + 190, "Sky Core OS Kurulum Sihirbazina Hos Geldiniz!", COLOR_TEXT_DARK);
    draw_string_graphics(panel_x + 80, panel_y + 220, "Sistemi hemen yapilandirmak icin asagidan bir mod secin.", COLOR_TEXT_DARK);
    
    // 6. Etkileşimli Kurulum Butonları
    // Buton A: Hızlı Kurulum
    draw_filled_rectangle(panel_x + 60, panel_y + 290, 230, 45, COLOR_ACTIVE_BLUE);
    draw_string_graphics(panel_x + 90, panel_y + 305, "Hizli Kurulum (Az)", COLOR_TEXT_LIGHT);
    
    // Buton B: Detaylı Kurulum
    draw_filled_rectangle(panel_x + 360, panel_y + 290, 230, 45, COLOR_DARK_BLUE);
    draw_string_graphics(panel_x + 385, panel_y + 305, "Detayli Kurulum (Baska)", COLOR_TEXT_LIGHT);
    
    // 7. Sol Alt Köşe - Sistem Sürüm Bilgisi Bilgi Kartı
    draw_filled_rectangle(20, SCREEN_HEIGHT - 70, 180, 45, COLOR_WIDGET_BG);
    draw_string_graphics(35, SCREEN_HEIGHT - 55, "SKY CORE OS v1.5", COLOR_TEXT_LIGHT);
}

/**
 * @brief ARAMA 2: Konum ve Bölge Saati Ayarları
 * Ekrana altın sarısı Türkiye Haritasını, pin konumunu ve saat girdilerini basar.
 */
void draw_ui_location_screen(void) {
    // 1. Arka plan tazeleme
    draw_vertical_gradient(COLOR_DEEP_PURPLE, COLOR_DARK_BLUE);
    
    // 2. Üst Büyük Bilgi Şeridi
    draw_filled_rectangle(0, 0, SCREEN_WIDTH, 60, COLOR_WIDGET_BG);
    draw_string_graphics(40, 22, "ASAMA 2: KONUM VE BOLGE SAATI AYARLARI", COLOR_TEXT_LIGHT);
    
    // 3. Sağ Taraf: Büyük Türkiye Haritası Panel Kutusu
    int map_x = 480;
    int map_y = 120;
    int map_w = 500;
    int map_h = 320;
    draw_filled_rectangle(map_x, map_y, map_w, map_h, COLOR_WIDGET_BG);
    draw_rectangle_outline(map_x, map_y, map_w, map_h, COLOR_WHITE);
    
    // Temsili Altın Sarısı Türkiye Haritası Kütlesi (Dikdörtgen bloklar ile harita matris simülasyonu)
    draw_filled_rectangle(map_x + 40, map_y + 80, 420, 140, COLOR_MAP_GOLD);
    draw_filled_rectangle(map_x + 20, map_y + 110, 50, 60, COLOR_MAP_GOLD); // Trakya Bölgesi
    
    // İstanbul Konum Pini (Kırmızı Nokta)
    draw_filled_rectangle(map_x + 80, map_y + 100, 10, 10, 0xFFFF0000); 
    draw_string_graphics(map_x + 95, map_y + 95, "Istanbul", COLOR_TEXT_LIGHT);
    
    // 4. Sol Taraf: Giriş Form Alanları
    int form_x = 50;
    int form_y = 140;
    draw_string_graphics(form_x, form_y, "Sistem Konumu:", COLOR_TEXT_LIGHT);
    draw_filled_rectangle(form_x, form_y + 20, 380, 40, COLOR_WHITE);
    draw_string_graphics(form_x + 15, form_y + 32, "Istanbul, Turkiye", COLOR_TEXT_DARK);
    
    draw_string_graphics(form_x, form_y + 90, "Bolge Saati Entegrasyonu:", COLOR_TEXT_LIGHT);
    draw_filled_rectangle(form_x, form_y + 110, 380, 40, COLOR_WHITE);
    draw_string_graphics(form_x + 15, form_y + 122, "GMT+03:00 (Yaz Saati Uygulamasi)", COLOR_TEXT_DARK);
    
    // 5. Alt Navigasyon Uyarı Şeridi
    draw_string_graphics(form_x, form_y + 220, "Devam etmek icin [ENTER] tusuna basin.", COLOR_GLOW_CYAN);
}

/**
 * @brief ARAMA 3: Giriş Bilgileri ve Kurulumu Tamamlama Ekranı
 * Büyük Gece Ayı sembolünü çizer ve "BAŞLAT" komut penceresini ekrana mühürler.
 */
void draw_ui_summary_screen(void) {
    // 1. Arka plan gradyanı
    draw_vertical_gradient(COLOR_DEEP_PURPLE, COLOR_DARK_BLUE);
    
    // 2. Ortadaki Özet Bildirim Penceresi
    int box_w = 600;
    int box_h = 300;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;
    draw_filled_rectangle(box_x, box_y, box_w, box_h, COLOR_WIDGET_BG);
    draw_rectangle_outline(box_x, box_y, box_w, box_h, COLOR_GLOW_CYAN);
    
    // 3. Büyük Ay Logosu Simülasyonu (33.png Gece Teması)
    int moon_x = box_x + 50;
    int moon_y = box_y + 90;
    draw_filled_rectangle(moon_x, moon_y, 80, 80, 0xFFFFF59D); // Parlak Sarı Hilal Alanı
    draw_filled_rectangle(moon_x + 30, moon_y, 50, 80, COLOR_DARK_BLUE); // Gölge kesimi
    
    // 4. Bilgilendirme Metinleri
    draw_string_graphics(box_x + 160, box_y + 80, "GIRIS VE TAMAMLAMA", COLOR_GLOW_CYAN);
    draw_string_graphics(box_x + 160, box_y + 120, "Giris bilgileriniz kontrol edildi.", COLOR_TEXT_LIGHT);
    draw_string_graphics(box_x + 160, box_y + 150, "Sky Core OS kullanima tamamen hazir!", COLOR_TEXT_LIGHT);
    
    // 5. Dev Kapanış Butonu: BAŞLAT
    int btn_w = 400;
    int btn_h = 50;
    int btn_x = box_x + (box_w - btn_w) / 2;
    int btn_y = box_y + 220;
    draw_filled_rectangle(btn_x, btn_y, btn_w, btn_h, 0xFF4CAF50); // Yeşil Başarı Butonu
    draw_rectangle_outline(btn_x, btn_y, btn_w, btn_h, COLOR_WHITE);
    draw_string_graphics(btn_x + 110, btn_y + 18, "KURULUMU TAMAMLA VE BASLAT", COLOR_TEXT_LIGHT);
}

/**
 * @brief ARAMA 4: Fırtına ve Ay Temalı Widget'lı Ana Masaüstü (Desktop)
 * Kurulum bittikten sonra açılan, widget'lar ve uygulama barındıran asıl masaüstü.
 */
void draw_ui_main_desktop(void) {
    // 1. Masaüstü Arka Planı (Derin Gece Mavisi Fırtına Teması)
    draw_vertical_gradient(0x02110222, 0x00021102);
    
    // 2. SOL ÜST: Devasa Hava Durumu ve Zaman Widget'ı
    int wd_x = 40;
    int wd_y = 40;
    draw_filled_rectangle(wd_x, wd_y, 340, 180, COLOR_WIDGET_BG);
    draw_rectangle_outline(wd_x, wd_y, 340, 180, COLOR_WHITE);
    
    // Saat ve Durum Verileri
    draw_string_graphics(wd_x + 20, wd_y + 30, "SAAT: 26:03", COLOR_GLOW_CYAN); // Vasiyet zaman damgası
    draw_string_graphics(wd_x + 20, wd_y + 70, "Konum: Esenyurt, Ist", COLOR_TEXT_LIGHT);
    draw_string_graphics(wd_x + 20, wd_y + 110, "Hava: 21C - Bulutlu & Firtina", COLOR_TEXT_LIGHT);
    
    // 3. SAĞ BAR: Uygulamalar Çekmecesi Paneli
    int bar_w = 200;
    int bar_x = SCREEN_WIDTH - bar_w;
    draw_filled_rectangle(bar_x, 0, bar_w, SCREEN_HEIGHT, COLOR_WIDGET_BG);
    draw_rectangle_outline(bar_x, 0, bar_w, SCREEN_HEIGHT, COLOR_WHITE);
    draw_string_graphics(bar_x + 35, 30, "UYGULAMALAR", COLOR_GLOW_CYAN);
    
    // Kısayol İkon Simülasyonları
    // İkon 1: Terminal
    draw_filled_rectangle(bar_x + 30, 80, 140, 40, COLOR_DARK_BLUE);
    draw_string_graphics(bar_x + 50, 92, ">_ Terminal", COLOR_TEXT_LIGHT);
    
    // İkon 2: Dosya Yöneticisi
    draw_filled_rectangle(bar_x + 30, 140, 140, 40, COLOR_DARK_BLUE);
    draw_string_graphics(bar_x + 45, 152, "Dosya Yonetimi", COLOR_TEXT_LIGHT);
    
    // İkon 3: Haritalar
    draw_filled_rectangle(bar_x + 30, 200, 140, 40, COLOR_DARK_BLUE);
    draw_string_graphics(bar_x + 55, 212, "Haritalar", COLOR_TEXT_LIGHT);
    
    // 4. ANA MASAÜSTÜ ALANI SİSTEM İKONLARI
    // Masaüstündeki "Kamera" veya "Ayarlar" Klasörleri
    draw_filled_rectangle(60, 300, 70, 70, COLOR_ACTIVE_BLUE);
    draw_string_graphics(65, 385, "Kamera.sys", COLOR_TEXT_LIGHT);
    
    draw_filled_rectangle(180, 300, 70, 70, COLOR_ACTIVE_BLUE);
    draw_string_graphics(190, 385, "Ayarlar", COLOR_TEXT_LIGHT);
    
    // 5. ALT DOCK PANELİ
    int dock_w = 600;
    int dock_h = 55;
    int dock_x = (SCREEN_WIDTH - bar_w - dock_w) / 2;
    int dock_y = SCREEN_HEIGHT - 75;
    draw_filled_rectangle(dock_x, dock_y, dock_w, dock_h, COLOR_WIDGET_BG);
    draw_rectangle_outline(dock_x, dock_y, dock_w, dock_h, COLOR_GLOW_CYAN);
    draw_string_graphics(dock_x + 60, dock_y + 20, "TUSUMANA BASINCA UYGULAMA CEKMECESI ACILSIN", COLOR_TEXT_LIGHT);
}

// ==============================================================================
// 🗺️ 9. UI DURUM GEÇİŞ YÖNETİCİSİ (STATE CONTROLLER)
// ==============================================================================

/**
 * @brief Aktif duruma göre ekranın tamamını temizleyip taze arayüzü basar.
 */
void refresh_system_display(void) {
    // Ekranı her geçişte temizle (Flicker/Gölge kalıntılarını önleme katmanı)
    fill_screen(COLOR_DARK_BLUE);
    
    switch (current_os_state) {
        case STATE_WELCOME:
            draw_ui_welcome_screen();
            break;
        case STATE_LOCATION:
            draw_ui_location_screen();
            break;
        case STATE_SUMMARY:
            draw_ui_summary_screen();
            break;
        case STATE_DESKTOP:
            draw_ui_main_desktop();
            break;
    }
}

/**
 * @brief ENTER'a basıldığında bir sonraki kurulum aşamasına atlatır.
 */
void advance_os_stage(void) {
    if (current_os_state < STATE_DESKTOP) {
        current_os_state++;
        refresh_system_display();
    }
}

/**
 * @brief BACKSPACE'e basıldığında kurulum aşamalarında geri gitmeyi sağlar.
 */
void regress_os_stage(void) {
    if (current_os_state > STATE_WELCOME && current_os_state != STATE_DESKTOP) {
        current_os_state--;
        refresh_system_display();
    }
}

// ==============================================================================
// 🚀 10. MAIN KERNEL ENTRY POINT (ANA ÇEKİRDEK GİRİŞ NOKTASI)
// ==============================================================================

void kernel_main(void* mboot_ptr, uint32_t magic) {
    // Multiboot parametrelerinin derleyici uyarısı vermemesi için kör kilitleme
    (void)mboot_ptr;
    (void)magic;
    
    // --------------------------------------------------------------------------
    // TERMINAL TAMAMEN ES GEÇİLİYOR - DOĞRUDAN LFB GRAFİK MODU ATEŞLENDİ
    // --------------------------------------------------------------------------
    // Grafik sürücü kartı ve hafıza katmanı ilklendiriliyor
    setup_init(); 
    
    // Çekirdeğin durum makinesini 1. Aşamaya (Hoş Geldiniz) ayarlıyoruz
    current_os_state = STATE_WELCOME;
    
    // Grafik belleğine pikselleri ilk kez basıyoruz
    refresh_system_display();

    // Giriş tamponunun bellek güvenliği için sıfırlanması
    for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;

    // ==========================================================================
    // ⌨️ 11. ULTRA POLLING KLAVYE MOTORU & SCANTABLE REAKSİYON DÖNGÜSÜ
    // ==========================================================================
    while (1) {
        // Intel 8042 PS/2 klavye kontrolcüsü durum yazmacı (Status Register) sorgulanıyor
        // Bit 0 = 1 ise çıktı tamponunda okunmayı bekleyen klavye verisi vardır.
        if (inb(0x64) & 1) { 
            // Veri portundan (0x60) donanımsal scancode okunuyor
            uint8_t scancode = inb(0x60);
            
            if (is_graphics_mode) {
                // Yakalanan donanımsal girdiyi eş zamanlı olarak yan modüllere (setup.c) besle
                setup_handle_input(scancode); 
                
                // Sadece tuşa basılma anını yakala (Make Code), parmak çekme kodlarını (Break Code) filtrele
                // x86 mimarisinde Break kodlarının 7. biti (0x80) her zaman 1'dir.
                if (!(scancode & 0x80)) {
                    switch (scancode) {
                        case 0x1C: // ENTER Tuşu Basıldı
                            // Kurulum adımlarında ileri git, 3. aşamadaysa canavar masaüstünü patlat!
                            advance_os_stage();
                            break;
                            
                        case 0x0E: // BACKSPACE Tuşu Basıldı
                            // Kurulum aşamalarında geriye dönük revizyon yapmayı sağlar
                            regress_os_stage();
                            break;
                            
                        default:
                            // Diğer tuşların basımını ASCII tamponuna yazma denemesi yapabilirsin
                            break;
                    }
                }
            } else {
                // Siyah terminal koruma yedek modu (Eğer grafik modu çökerse devreye giren katman)
                if (!(scancode & 0x80)) {
                    if (scancode == 0x1F) { // 'S' Tuşu Scancode'u
                        if (cmd_idx < 14) { 
                            cmd_buffer[cmd_idx++] = 's'; 
                            print_string("s"); 
                        }
                    } else if (scancode == 0x1C) { // ENTER Tuşu
                        cmd_buffer[cmd_idx] = '\0';
                        handle_cli_command(cmd_buffer);
                        
                        // Tamponu sıfırla
                        for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
                        cmd_idx = 0;
                    }
                }
            }
        }
        
        // İşlemcinin (CPU) boş döngüde aşırı ısınmasını engellemek, saat döngülerini optimize etmek
        // ve VirtualBox/QEMU thread'lerini rahatlatmak için Assembly "pause" talimatı gönderiliyor.
        __asm__ volatile("pause");
    }
}
