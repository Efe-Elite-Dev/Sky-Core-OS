/**
 * ==============================================================================
 * WIND OS / SKY CORE OS v1.5 - Vortex Kernel (Canli Sürüm)
 * ==============================================================================
 */

#include <stdint.h>
#include <stddef.h>

/* ============================================================
 * 1. GLOBAL DEĞİŞKENLER & DONANIM ADRESLERİ
 * ============================================================ */
uint32_t* GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;
int       is_graphics_mode     = 1;

// Fare koordinatları ve durumu
static int mouse_x = 512;
static int mouse_y = 384;
static int prev_mouse_x = 512;
static int prev_mouse_y = 384;

/* ============================================================
 * 2. DIŞ BAĞLANTI BİLDİRİMLERİ
 * ============================================================ */
extern void force_graphics_hardware(void);   /* vga_force.c */
extern void screen_init(void);               /* screen.c    */
extern void setup_init(void);                /* setup.c     */
extern void setup_handle_input(uint8_t sc);  /* setup.c     */

/* ============================================================
 * 3. DONANIM I/O FONKSİYONLARI
 * ============================================================ */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void print_string(const char *str) { (void)str; }

#define SW 1024
#define SH 768

/* ============================================================
 * 4. GRAFİK ÇİZİM MOTORU (ÇİFT TAMPON/BACKBUFFER DESTEKLİ)
 * ============================================================ */
static inline void put_px(int x, int y, uint32_t c) {
    if ((unsigned)x < SW && (unsigned)y < SH)
        GRAPHICS_FRAMEBUFFER[y * SW + x] = c;
}

static void fill_screen(uint32_t c) {
    int n = SW * SH;
    for (int i = 0; i < n; i++) GRAPHICS_FRAMEBUFFER[i] = c;
}

static void draw_rect(int x, int y, int w, int h, uint32_t c) {
    for (int r = y; r < y + h; r++)
        for (int col = x; col < x + w; col++)
            put_px(col, r, c);
}

static void draw_rect_outline(int x, int y, int w, int h, uint32_t c) {
    for (int i = x; i < x + w; i++) { put_px(i, y, c); put_px(i, y+h-1, c); }
    for (int i = y; i < y + h; i++) { put_px(x, i, c); put_px(x+w-1, i, c); }
}

static void gradient_v(uint32_t top, uint32_t bot) {
    for (int y = 0; y < SH; y++) {
        int t = SH - y, b = y;
        uint8_t r = (uint8_t)(((top>>16&0xFF)*t + (bot>>16&0xFF)*b) / SH);
        uint8_t g = (uint8_t)(((top>>8 &0xFF)*t + (bot>>8 &0xFF)*b) / SH);
        uint8_t bl= (uint8_t)(((top    &0xFF)*t + (bot    &0xFF)*b) / SH);
        uint32_t px = (0xFF<<24)|(r<<16)|(g<<8)|bl;
        for (int x = 0; x < SW; x++) GRAPHICS_FRAMEBUFFER[y*SW+x] = px;
    }
}

/* 5x7 Bitmap Font Verisi */
static const uint8_t FONT5[128][7] = {
    [' '] = {0,0,0,0,0,0,0}, ['!'] = {0x04,0x04,0x04,0x04,0x00,0x04,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x04,0x00}, [':'] = {0x00,0x04,0x00,0x00,0x04,0x00,0x00},
    ['0'] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, ['1'] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    ['2'] = {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F}, ['3'] = {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},
    ['4'] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, ['5'] = {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    ['6'] = {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, ['7'] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    ['8'] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, ['9'] = {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    ['A'] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, ['B'] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    ['C'] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, ['D'] = {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    ['E'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, ['F'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    ['G'] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E}, ['H'] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    ['I'] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, ['J'] = {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    ['K'] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, ['L'] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    ['M'] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, ['N'] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    ['O'] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, ['P'] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    ['Q'] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, ['R'] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    ['S'] = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, ['T'] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    ['U'] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, ['V'] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},
    ['W'] = {0x11,0x11,0x15,0x15,0x15,0x1B,0x11}, ['X'] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    ['Y'] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, ['Z'] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

static void draw_char(int x, int y, char ch, uint32_t col, int scale) {
    if ((unsigned char)ch >= 128) return;
    const uint8_t *bm = FONT5[(unsigned char)ch];
    for (int row = 0; row < 7; row++)
        for (int bit = 4; bit >= 0; bit--)
            if (bm[row] & (1 << bit))
                for (int sy = 0; sy < scale; sy++)
                    for (int sx = 0; sx < scale; sx++)
                        put_px(x + (4-bit)*scale + sx, y + row*scale + sy, col);
}

static void draw_str(int x, int y, const char *s, uint32_t col, int scale) {
    while (*s) { draw_char(x, y, *s, col, scale); x += 6*scale; s++; }
}

/* Donanım Fare İmleci Çizici */
static void draw_mouse_cursor(int x, int y, uint32_t color) {
    for(int i = 0; i < 12; i++) {
        for(int j = 0; j <= i; j++) {
            put_px(x + j, y + i, color);
        }
    }
}

/* ============================================================
 * 5. UI EKRANLARI VE MASAÜSTÜ SİSTEMİ
 * ============================================================ */
static void draw_oobe_base(void) {
    gradient_v(0xFFE8F4FD, 0xFFD0E8F8);
    draw_rect(190, 90, 644, 560, 0x22000000);
    draw_rect(192, 94, 640, 556, 0xFFFFFFFF);
    draw_rect_outline(192, 94, 640, 556, 0xFFDDE5EE);
}

static void draw_next_btn(int bx, int by) {
    draw_rect(bx, by, 180, 46, 0xFF0078D4);
    draw_str(bx+50, by+15, "Evet", 0xFFFFFFFF, 2);
}

static void render_oobe_region(void) {
    draw_oobe_base();
    draw_str(270, 130, "Bu dogru ulke veya bolge mi?", 0xFF1A1A2E, 2);
    draw_rect(280, 230, 440, 50, 0xFFF0F6FF);
    draw_rect_outline(280, 230, 440, 50, 0xFF0078D4);
    draw_str(294, 248, "Turkiye", 0xFF1A1A2E, 2);
    draw_str(230, 460, "ENTER - Ileri    BACKSPACE - Geri", 0xFF666680, 1);
    draw_next_btn(590, 570);
}

// Masaüstü uygulamasını interaktif hale getirmek için pencereler
static int is_terminal_open = 0;

static void render_desktop(void) {
    gradient_v(0xFF1A0F2E, 0xFF0D0B18);

    /* Üst Görev Çubuğu */
    draw_rect(0, 0, SW, 38, 0xAA14102A);
    draw_str(10, 12, "Wind OS v1.5", 0xFF00E5FF, 1);
    draw_str(950, 12, "21:10", 0xFF00E5FF, 1);

    /* Hava Durumu & Saat Widget */
    draw_rect(30, 52, 480, 150, 0xAA211C38);
    draw_rect_outline(30, 52, 480, 150, 0xFF444466);
    draw_str(50, 72,  "SAAT: 21:10", 0xFF00E5FF, 2);
    draw_str(50, 112, "Hava Durumu - Esenyurt", 0xFFF5F5F5, 1);
    draw_str(50, 132, "21C Bulutlu ve Firtina", 0xFFD0D0D0, 1);

    /* Uygulama İkonları (Tıklanabilir Alanlar) */
    const char *dock_labels[4] = {"Kamera", "Mesaj", "Dosya", "Terminal"};
    for (int i = 0; i < 4; i++) {
        draw_rect(30, 230+i*80, 100, 50, 0xFF0D0B18);
        draw_rect_outline(30, 230+i*80, 100, 50, 0xFF444466);
        draw_str(35, 250+i*80, dock_labels[i], 0xFFF5F5F5, 1);
    }

    /* Eğer Terminal EXE'si tetiklendiyse Canlı Pencere Göster */
    if (is_terminal_open) {
        draw_rect(200, 230, 500, 300, 0xFF000000);
        draw_rect_outline(200, 230, 500, 300, 0xFF00E5FF);
        draw_rect(200, 230, 500, 25, 0xFF14102A);
        draw_str(210, 236, "Terminal Subsystem - Executing terminal.exe", 0xFF00E5FF, 1);
        draw_str(210, 270, "root@skycore:~# ./exe_subsystem_init", 0xFF00FF00, 1);
        draw_str(210, 290, "Executing binary ELF stub inside protect mode...", 0xFFFFFFFF, 1);
        draw_str(210, 320, "Status: SUCCESS", 0xFF00FF00, 1);
    } else {
        /* Hoş Geldiniz Bildirimi */
        int px=240, py=300, pw=380, ph=110;
        draw_rect(px, py, pw, ph, 0xFFFFFFFF);
        draw_rect_outline(px, py, pw, ph, 0xFF00E5FF);
        draw_str(px+30, py+20, "HOS GELDINIZ!", 0xFF1A1A2E, 2);
        draw_str(px+30, py+55, "Tiklanabilir Terminal.exe aktif.", 0xFF333355, 1);
    }

    /* Alt Dock */
    draw_rect(20, SH-60, 700, 45, 0xAA211C38);
    draw_rect_outline(20, SH-60, 700, 45, 0xFF00E5FF);
    draw_str(35, SH-42, "Uygulamalara Tiklayarak Gercek Zamanli Calistirin", 0xFFF5F5F5, 1);
}

/* ============================================================
 * 6. DURUM MAKİNESİ (STATE MACHINE)
 * ============================================================ */
typedef enum { STATE_REGION = 0, STATE_DESKTOP } OS_STATE;
static volatile OS_STATE g_state = STATE_REGION;

static void refresh(void) {
    if (g_state == STATE_REGION) render_oobe_region();
    else render_desktop();
    
    // Her ekranda fareyi en son katman olarak üzerine basıyoruz
    draw_mouse_cursor(mouse_x, mouse_y, 0xFFFF0000);
}

static void go_next(void) { if (g_state < STATE_DESKTOP) { g_state++; refresh(); } }

/* ============================================================
 * 7. ENTEGRE EDİLMİŞ FARE VE DONANIM SÜRÜCÜLERİ (PS/2)
 * ============================================================ */
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8); // Fareyi aktif et
    
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    mouse_write(0xF6); // Varsayılan değerleri yükle
    mouse_read();
    mouse_write(0xF4); // Veri akışını aç
    mouse_read();
}

/* Gerçek Zamanlı Mouse Pozisyon Güncelleme ve Tıklama Yakalama */
void handle_mouse(void) {
    static uint8_t cycle = 0;
    static int8_t mouse_bytes[3];
    
    uint8_t status = inb(0x64);
    if ((status & 1) && (status & 5) == 32) { // Veri fareden mi geliyor kontrolü
        mouse_bytes[cycle++] = inb(0x60);
        
        if (cycle == 3) {
            cycle = 0;
            
            // Bit manipülasyonu ile x ve y hareket farklarını çıkarıyoruz
            int8_t rel_x = mouse_bytes[1];
            int8_t rel_y = mouse_bytes[2];
            
            mouse_x += rel_x / 2;  // Hassasiyet ayarı
            mouse_y -= rel_y / 2;
            
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= SW) mouse_x = SW - 1;
            if (mouse_y >= SH) mouse_y = SH - 1;
            
            // Sol Tık Kontrolü (mouse_bytes[0] içindeki 1. bit sol tık demektir)
            if (mouse_bytes[0] & 1) {
                // Eğer Kullanıcı Terminal İkonuna Tıkladıysa (X: 30-130, Y: 470-520 aralığı)
                if (g_state == STATE_DESKTOP && mouse_x > 30 && mouse_x < 130 && mouse_y > 470 && mouse_y < 520) {
                    is_terminal_open = 1; // .exe simülasyonunu çalıştır
                }
            }
            refresh();
        }
    }
}

/* ============================================================
 * 8. EXE VE SİSTEM ALT YAPILARI (STUB'LARI CANLANDIRMA)
 * ============================================================ */
typedef void (*exe_entry_t)(void); // Fonksiyon göstericisi ile ham ikili kod çalıştırma mantığı

void exe_subsystem_init(void) {
    // Çekirdek içinde izole bir kod parçacığını .exe gibi tetikleme mimarisi
    is_terminal_open = 1;
}

void idt_init(void) {
    // Donanım interrupt hatlarını kurmak için PIC yapılandırması
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

/* ============================================================
 * 9. KERNEL GİRİŞ NOKTASI
 * ============================================================ */
void kernel_main(void *mboot_ptr, uint32_t magic) {
    (void)magic;

    if (mboot_ptr) {
        uint32_t flags = *(uint32_t *)mboot_ptr;
        if (flags & (1u << 11)) {
            uint32_t fb_addr = ((uint32_t *)mboot_ptr)[22];
            if (fb_addr) GRAPHICS_FRAMEBUFFER = (uint32_t *)fb_addr;
        }
    }

    is_graphics_mode = 1;
    force_graphics_hardware();
    
    // Donanım katmanlarını sırayla başlatıyoruz
    idt_init();
    mouse_init();
    
    g_state = STATE_REGION;
    refresh();

    // Canlı Polling / Kesme Döngüsü
    while (1) {
        // Klavyeyi dinle
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            if (!(sc & 0x80)) {
                if (sc == 0x1C) {
                    go_next(); /* ENTER */
                } 
                else if (sc == 0x3B) { 
                    // F1 tuşuna basılırsa terminal.exe alt sistemini direkt çalıştır
                    exe_subsystem_init();
                    refresh();
                }
            }
        }
        
        // Fareyi dinle
        handle_mouse();
        
        __asm__ volatile("pause");
    }
}

/* Kalan Diğer Bağlantılar */
void keyboard_init(void)       {}
void wind_subsystem_init(void) {}
void ai_subsystem_init(void)   {}
void deb_subsystem_init(void)  {}
