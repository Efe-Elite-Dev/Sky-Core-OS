/**
 * ==============================================================================
 * 🌟 WIND OS / SKY CORE OS v1.5 - MONOLITHIC GUI ULTRA KERNEL 🌟
 * ==============================================================================
 */

#include <stdint.h>
#include <stddef.h>
#include "globals.h" // Ortak değişken tanımları

// Global Değişken Tanımları (Burada static yok, yani diğer dosyalar görebilir)
uint32_t* GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;
int is_graphics_mode = 1;

// Harici fonksiyon bildirimleri
extern void force_graphics_hardware(void);

// 🪐 1. SİSTEM DURUM MAKİNESİ
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

// 🛠️ 2. LOW-LEVEL I/O PORT SÜRÜCÜSÜ
static inline uint8_t sky_inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// 🖼️ 3. ALGORİTMİK GÖRSEL MOTORU
static void generate_sky_oobe_bg(uint32_t* buffer, uint32_t highlight_color) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        uint8_t r = 240 - (y * 40 / SCREEN_HEIGHT);
        uint8_t g = 244 - (y * 30 / SCREEN_HEIGHT);
        uint8_t b = 248 - (y * 20 / SCREEN_HEIGHT);
        uint32_t bg_pixel = (0xFF << 24) | (r << 16) | (g << 8) | b;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int idx = y * SCREEN_WIDTH + x;
            if (x > 180 && x < 844 && y > 100 && y < 668) {
                if (highlight_color != 0 && x > 250 && x < 400 && y > 300 && y < 450)
                    buffer[idx] = highlight_color;
                else
                    buffer[idx] = 0xFFFFFFFF;
            } else {
                buffer[idx] = bg_pixel;
            }
        }
    }
}

static void generate_sky_desktop_bg(uint32_t* buffer) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        uint8_t r = 13 - (y * 8 / SCREEN_HEIGHT);
        uint8_t g = 11 - (y * 6 / SCREEN_HEIGHT);
        uint8_t b = 24 - (y * 10 / SCREEN_HEIGHT);
        uint32_t dark_bg = (0xFF << 24) | (r << 16) | (g << 8) | b;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int idx = y * SCREEN_WIDTH + x;
            if (x >= SCREEN_WIDTH - 240 && y > 40 && y < SCREEN_HEIGHT - 80) buffer[idx] = 0xAA211C38;
            else if (y >= SCREEN_HEIGHT - 80 && x > 40 && x < 740) buffer[idx] = 0xAA1C1830;
            else if (x > 40 && x < 540 && y > 40 && y < 240) buffer[idx] = 0xAA262142;
            else buffer[idx] = dark_bg;
        }
    }
}

// 🎨 4. GÜNCELLEME MOTORU
static void update_sky_ui_layer(void) {
    if (GRAPHICS_FRAMEBUFFER == NULL) return;
    switch (current_sky_state) {
        case SKY_STATE_OOBE_REGION:    generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFF2575FC); break;
        case SKY_STATE_OOBE_KEYBOARD:  generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFF4A5568); break;
        case SKY_STATE_OOBE_NETWORK:   generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFF00E5FF); break;
        case SKY_STATE_OOBE_NAME:      generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFF9F7AEA); break;
        case SKY_STATE_OOBE_PRIVACY:   generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFF38A169); break;
        case SKY_STATE_OOBE_CUSTOMIZE: generate_sky_oobe_bg(GRAPHICS_FRAMEBUFFER, 0xFFED8936); break;
        case SKY_STATE_WIND_DESKTOP:   generate_sky_desktop_bg(GRAPHICS_FRAMEBUFFER); break;
    }
}

// Navigasyon (İsimleri çakışmasın diye değiştirildi)
static void sky_process_next(void) {
    if (current_sky_state < SKY_STATE_WIND_DESKTOP) {
        current_sky_state++;
        update_sky_ui_layer();
    }
}

static void sky_process_prev(void) {
    if (current_sky_state > SKY_STATE_OOBE_REGION) {
        current_sky_state--;
        update_sky_ui_layer();
    }
}

// 🚀 5. ANA GİRİŞ NOKTASI
void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)magic;
    if (mboot_ptr != NULL) {
        uint32_t flags = *(uint32_t*)mboot_ptr;
        if (flags & (1 << 11)) {
            uint32_t* vbe_mode_info = (uint32_t*)((uint8_t*)mboot_ptr + 72);
            uint32_t addr = *vbe_mode_info;
            if (addr != 0) GRAPHICS_FRAMEBUFFER = (uint32_t*)addr;
        }
    }

    force_graphics_hardware();
    update_sky_ui_layer();

    while (1) {
        if (sky_inb(0x64) & 1) {
            uint8_t code = sky_inb(0x60);
            if (!(code & 0x80)) {
                if (code == 0x1C) sky_process_next();
                else if (code == 0x0E) sky_process_prev();
            }
        }
        __asm__ volatile("pause");
    }
}

// Stublar
void idt_init(void) {}
void keyboard_init(void) {}
void mouse_init(void) {}
void wind_subsystem_init(void) {}
void exe_subsystem_init(void) {}
void ai_subsystem_init(void) {}
void deb_subsystem_init(void) {}
