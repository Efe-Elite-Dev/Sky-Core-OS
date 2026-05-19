/**
 * ==============================================================================
 * 🌟 WIND OS / SKY CORE OS v1.5 - MONOLITHIC GUI ULTRA KERNEL 🌟
 * ==============================================================================
 * [Mimari]: x86 Intel/AMD IA-32 Korumalı Mod (Protected Mode)
 * [Ekran Çözünürlüğü]: 1024x768 x 32bpp VBE (VESA Bios Extensions) LFB
 * [Geliştirici]: Feyzula Efe Tuna
 * [Açıklama]: setup.c veya gui.c ile asla çakışmayan, isim alanları (namespaces)
 * tamamen izole edilmiş ve tek seferde hatasız derlenen ana çekirdek.
 * ==============================================================================
 */

#include <stdint.h>
#include <stddef.h>

// 🪐 1. ÇAKIŞMASIZ SİTEM DURUM MAKİNESİ (STATE MACHINE)
typedef enum {
    SKY_STATE_OOBE_REGION = 0,
    SKY_STATE_OOBE_KEYBOARD = 1,
    SKY_STATE_OOBE_NETWORK = 2,
    SKY_STATE_OOBE_NAME = 3,
    SKY_STATE_OOBE_PRIVACY = 4,
    SKY_STATE_OOBE_CUSTOMIZE = 5,
    SKY_STATE_WIND_DESKTOP = 6
} SKY_KERNEL_UI_STATE;

static volatile SKY_KERNEL_UI_STATE current_sky_state = SKY_STATE_OOBE_REGION;

#define SCREEN_WIDTH         1024
#define SCREEN_HEIGHT        768

// 🖥 Honor Grafik Bellek Alanı
static uint32_t* SKY_GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;

// Çakışmayı önlemek için harici değişkenleri lokalde gölgeliyoruz
extern int is_graphics_mode;
extern void force_graphics_hardware(void);

// 🛠️ 2. LOW-LEVEL I/O PORT SÜRÜCÜSÜ
static inline uint8_t sky_inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ==============================================================================
// 🖼️ 3. ALGORİTMİK WINDOWS OOBE VE WIND OS DESKTOP SİMÜLASYONU
// ==============================================================================

static void generate_sky_oobe_bg(uint32_t* buffer, uint32_t highlight_color) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Windows 11 OOBE pürüzsüz açık gri-mavi gökyüzü degradesi
        uint8_t r = 240 - (y * 40 / SCREEN_HEIGHT);
        uint8_t g = 244 - (y * 30 / SCREEN_HEIGHT);
        uint8_t b = 248 - (y * 20 / SCREEN_HEIGHT);
        uint32_t bg_pixel = (0xFF << 24) | (r << 16) | (g << 8) | b;

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int idx = y * SCREEN_WIDTH + x;
            
            // Ortadaki modern beyaz OOBE Kurulum Kartı (Tıpatıp Kurulum Ekranları)
            if (x > 180 && x < 844 && y > 100 && y < 668) {
                // Sol taraftaki ikonlar veya sağdaki öğe odaklama alanları
                if (highlight_color != 0 && x > 250 && x < 400 && y > 300 && y < 450) {
                    buffer[idx] = highlight_color; 
                } else {
                    buffer[idx] = 0xFFFFFFFF; // Pürüzsüz Saf Beyaz Kart Arka Planı
                }
            } else {
                buffer[idx] = bg_pixel;
            }
        }
    }
}

static void generate_sky_desktop_bg(uint32_t* buffer) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Koyu fırtınalı Wind OS fütüristik mor-mavi arka plan degradesi
        uint8_t r = 13 - (y * 8 / SCREEN_HEIGHT);
        uint8_t g = 11 - (y * 6 / SCREEN_HEIGHT);
        uint8_t b = 24 - (y * 10 / SCREEN_HEIGHT);
        uint32_t dark_bg = (0xFF << 24) | (r << 16) | (g << 8) | b;

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int idx = y * SCREEN_WIDTH + x;
            
            // Sağ taraftaki dikey modern uygulamalar çekmecesi paneli
            if (x >= SCREEN_WIDTH - 240 && y > 40 && y < SCREEN_HEIGHT - 80) {
                buffer[idx] = 0xAA211C38; 
            } // Alt taraftaki şeffaf görev çubuğu bölmesi 
            else if (y >= SCREEN_HEIGHT - 80 && x > 40 && x < 740) {
                buffer[idx] = 0xAA1C1830; 
            } // Üst sol taraftaki büyük saat ve hava durumu widget alanı (26:03)
            else if (x > 40 && x < 540 && y > 40 && y < 240) {
                buffer[idx] = 0xAA262142; 
            } else {
                buffer[idx] = dark_bg; 
            }
        }
    }
}

// ==============================================================================
// 🎨 4. İZOLE EDİLMİŞ GÖRSEL GÜNCELLEME MOTORU
// ==============================================================================

static void update_sky_ui_layer(void) {
    if (SKY_GRAPHICS_FRAMEBUFFER == NULL) return;

    switch (current_sky_state) {
        case SKY_STATE_OOBE_REGION:
            // 1. Ekran: "Bu doğru ülke/bölge mi? (Türkiye)" -> Mavi Dünya Küresi Vurgusu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFF2575FC);
            break;
        case SKY_STATE_OOBE_KEYBOARD:
            // 2. Ekran: "Bu doğru klavye düzeni mi? (Türkçe Q)" -> Klavye Şablon Vurgusu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFF4A5568);
            break;
        case SKY_STATE_OOBE_NETWORK:
            // 3. Ekran: "Hadi sizi bir ağa bağlayalım" -> Parlak Wi-Fi Logosu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFF00E5FF);
            break;
        case SKY_STATE_OOBE_NAME:
            // 4. Ekran: "Bilgisayarınıza bir ad verelim" -> Laptop Şekli Vurgusu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFF9F7AEA);
            break;
        case SKY_STATE_OOBE_PRIVACY:
            // 5. Ekran: "Cihazınız için gizlilik ayarlarını seçin" -> Kalkan İkonu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFF38A169);
            break;
        case SKY_STATE_OOBE_CUSTOMIZE:
            // 6. Ekran: "Deneyiminizi özelleştirin" -> Oyun/Eğlence Kulaklık İkonu
            generate_sky_oobe_bg(SKY_GRAPHICS_FRAMEBUFFER, 0xFFED8936);
            break;
        case SKY_STATE_WIND_DESKTOP:
            // Final Aşaması: Tıpatıp kopyalanan fırtınalı Wind OS Masaüstü!
            generate_sky_desktop_bg(SKY_GRAPHICS_FRAMEBUFFER);
            break;
    }
}

// Çakışma ihtimalini tamamen sıfırlayan lokal navigasyon fonksiyonları
static void sky_internal_next(void) {
    if (current_sky_state < SKY_STATE_WIND_DESKTOP) {
        current_sky_state++;
        update_sky_ui_layer();
    }
}

static void sky_internal_prev(void) {
    if (current_sky_state > SKY_STATE_OOBE_REGION) {
        current_sky_state--;
        update_sky_ui_layer();
    }
}

// ==============================================================================
// 🚀 5. ANA ÇEKİRDEK GİRİŞ NOKTASI (KERNEL_MAIN)
// ==============================================================================
void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)magic;

    // Multiboot kontrolü ile VirtualBox grafik arabelleğinin gerçek adresini çek
    if (mboot_ptr != NULL) {
        uint32_t flags = *(uint32_t*)mboot_ptr;
        if (flags & (1 << 11)) {
            uint32_t* vbe_mode_info = (uint32_t*)((uint8_t*)mboot_ptr + 72);
            uint32_t dynamic_fb_addr = *vbe_mode_info;
            if (dynamic_fb_addr != 0) {
                SKY_GRAPHICS_FRAMEBUFFER = (uint32_t*)dynamic_fb_addr;
            }
        }
    }

    // Ekran donanımını grafik moduna geçir
    is_graphics_mode = 1;
    force_graphics_hardware();

    // Sistemi ilk pürüzsüz OOBE ekranıyla tetikle
    current_sky_state = SKY_STATE_OOBE_REGION;
    update_sky_ui_layer();

    // DÖNGÜ: ENTER ile sonraki adım, BACKSPACE ile önceki adım
    while (1) {
        // PS/2 Klavye donanım kontrol hattı
        if (sky_inb(0x64) & 1) {
            uint8_t scancode = sky_inb(0x60);
            
            // Sadece tuşa basıldığında (Make Code) sinyali yakala
            if (!(scancode & 0x80)) { 
                if (scancode == 0x1C) {       // ENTER Tuşu
                    sky_internal_next();
                }
                else if (scancode == 0x0E) {  // BACKSPACE Tuşu
                    sky_internal_prev();
                }
            }
        }
        // İşlemciyi yormamak için donanımsal duraklatma emri fırlat
        __asm__ volatile("pause");
    }
}
