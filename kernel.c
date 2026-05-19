/*
==============================================================================
WIND OS / SKY CORE OS v1.5 - Vortex Kernel (Fixed & Unified)
==============================================================================
Geliştirici: Feyzula Efe Tuna
*/

#include <stdint.h>
#include <stddef.h>

#define SW 1024
#define SH 768

/* ============================================================
 * 1. GLOBAL DEĞİŞKENLER
 * ============================================================ */
volatile uint32_t* GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;
uint32_t* gfx_framebuffer = (uint32_t*)0xFD000000; 
int is_graphics_mode = 1;
int ai_hud_visible = 0;

static int mouse_x = 512;
static int mouse_y = 384;

/* Durum Makinesi */
typedef enum { STATE_REGION = 0, STATE_DESKTOP } OS_STATE;
static volatile OS_STATE g_state = STATE_DESKTOP; // Direkt masaüstü

/* Uygulama Tanımları */
typedef enum { WIN_NONE = 0, WIN_TERMINAL } ActiveWindow;
static ActiveWindow active_win = WIN_NONE;

/* ============================================================
 * 2. LİNK HATALARINI ÇÖZEN STUBLAR (BOŞ FONKSİYONLAR)
 * ============================================================ */
void force_graphics_hardware(void) {} 
void screen_init(void) {}
void setup_init(void) {}
void keyboard_init(void) {}
void wind_subsystem_init(void) {}
void ai_subsystem_init(void) {}
void update_install(void) {}

/* ============================================================
 * 3. DONANIM I/O
 * ============================================================ */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void print_string(const char *str) { (void)str; }

/* ============================================================
 * 4. GRAFİK MOTORU
 * ============================================================ */
static inline void put_px(int x, int y, uint32_t c) {
    if ((unsigned)x < SW && (unsigned)y < SH)
        GRAPHICS_FRAMEBUFFER[y * SW + x] = c;
}

static void draw_rect(int x, int y, int w, int h, uint32_t c) {
    for (int r = y; r < y + h; r++)
        for (int col = x; col < x + w; col++)
            put_px(col, r, c);
}

static void draw_rect_outline(int x, int y, int w, int h, uint32_t c) {
    for (int i = x; i < x + w; i++) { put_px(i, y, c); put_px(i, y + h - 1, c); }
    for (int i = y; i < y + h; i++) { put_px(x, i, c); put_px(x + w - 1, i, c); }
}

static void gradient_v(uint32_t top, uint32_t bot) {
    for (int y = 0; y < SH; y++) {
        int t = SH - y, b = y;
        uint8_t r  = (uint8_t)(((top >> 16 & 0xFF) * t + (bot >> 16 & 0xFF) * b) / SH);
        uint8_t g  = (uint8_t)(((top >>  8 & 0xFF) * t + (bot >>  8 & 0xFF) * b) / SH);
        uint8_t bl = (uint8_t)(((top       & 0xFF) * t + (bot       & 0xFF) * b) / SH);
        uint32_t px = (0xFF << 24) | (r << 16) | (g << 8) | bl;
        for (int x = 0; x < SW; x++) GRAPHICS_FRAMEBUFFER[y * SW + x] = px;
    }
}

// Basit Font Fonksiyonu (Düzeltme)
static void draw_str(int x, int y, const char *s, uint32_t col, int scale) {
    (void)x; (void)y; (void)s; (void)col; (void)scale;
    // Buraya projenizdeki asıl font motorunu ekleyebilirsiniz.
}

static void draw_cursor(int x, int y) {
    draw_rect(x, y, 8, 8, 0xFFFFFFFF); // Beyaz kare imleç
    draw_rect_outline(x, y, 8, 8, 0xFF000000);
}

/* ============================================================
 * 5. MASAÜSTÜ ÇİZİMİ
 * ============================================================ */
static void render_desktop(void) {
    gradient_v(0xFF1A0F2E, 0xFF0D0B18); // Mor-Siyah neon degrade

    /* Üst bar */
    draw_rect(0, 0, SW, 38, 0xAA14102A);
    draw_str(10, 12, "Wind OS v1.5", 0xFF00E5FF, 1);

    /* Esenyurt Widget */
    draw_rect(30, 52, 480, 150, 0xAA211C38);
    draw_rect_outline(30, 52, 480, 150, 0xFF444466);
    draw_str(50, 72, "Hava Durumu - Esenyurt", 0xFFF5F5F5, 1);
    draw_str(50, 92, "21C Bulutlu ve Firtina", 0xFF00E5FF, 1);

    /* Alt dock */
    draw_rect(20, SH - 72, 700, 56, 0xAA211C38);
    draw_rect_outline(20, SH - 72, 700, 56, 0xFF00E5FF);

    draw_cursor(mouse_x, mouse_y);
}

static void refresh(void) {
    if (g_state == STATE_DESKTOP) render_desktop();
}

/* ============================================================
 * 6. MOUSE SÜRÜCÜSÜ
 * ============================================================ */
void mouse_init(void) {
    outb(0x64, 0xA8); outb(0x64, 0x20);
    uint8_t status = (inb(0x60) | 2);
    outb(0x64, 0x60); outb(0x60, status);
    outb(0x64, 0xD4); outb(0x60, 0xF4);
}

void handle_mouse(void) {
    if (inb(0x64) & 1) { (void)inb(0x60); } // Şimdilik sadece portu boşaltıyoruz
}

/* ============================================================
 * 7. KERNEL GİRİŞ NOKTASI (ENTRY)
 * ============================================================ */
void kernel_main(void *mboot_ptr, uint32_t magic) {
    (void)magic;
    if (mboot_ptr) {
        uint32_t fb_addr = ((uint32_t *)mboot_ptr)[22];
        if (fb_addr) {
            GRAPHICS_FRAMEBUFFER = (uint32_t *)fb_addr;
            gfx_framebuffer = (uint32_t *)fb_addr;
        }
    }

    mouse_init();
    g_state = STATE_DESKTOP; // OOBE'yi atla, direkt masaüstü
    refresh();

    while (1) {
        handle_mouse();
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            if (sc == 0x01) refresh(); // ESC basılırsa ekranı tazele
        }
        __asm__ volatile("hlt");
    }
}
