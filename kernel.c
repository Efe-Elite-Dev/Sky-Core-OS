#include <stdint.h>
#include <stddef.h>

// ==============================================================================
// 🪐 SKY CORE OS MİMARİ TANIMLAMALARI & DURUM MAKİNESİ (STATE MACHINE)
// ==============================================================================

// Sistem durumlarını yöneten ana enum yapısı.
// Tasarladığın arayüz akışını milimetrik olarak kontrol eder.
typedef enum {
    STATE_WELCOME = 0,    // Arama 1: İlk Kurulum Ekranı - Hoş Geldiniz
    STATE_LOCATION = 1,   // Arama 2: Konum & Saat Ayarlama (Türkiye Haritası)
    STATE_SUMMARY = 2,    // Arama 3: Giriş Bilgileri & Tamamlama
    STATE_DESKTOP = 3     // Arama 4: Fırtına & Ay Temalı Widget'lı Masaüstü!
} OS_UI_STATE;

// Global Sistem Durumu Değişkeni
volatile OS_UI_STATE current_os_state = STATE_WELCOME;

// ==============================================================================
// 🖥️ DIŞ DÜNYA BAĞLANTILARI (STUB / PROTOTİPLER)
// ==============================================================================
// setup.c içindeki grafik motorunun ve UI elementlerinin fonksiyon bildirimleri.
// Bu fonksiyonlar setup.c veya setup_ui.c içinde donanım piksellerini çizer.

extern void setup_init(void);
extern void setup_handle_input(uint8_t scancode);

// Çekirdeğin ekran kartı tamponuna doğrudan pikselleri basabilmesi için
// durum bazlı tetikleme fonksiyonları:
extern void draw_ui_welcome_screen(void);   // Hoş geldiniz ekranını çizer
extern void draw_ui_location_screen(void);  // Türkiye haritası ve saat ayarını çizer
extern void draw_ui_summary_screen(void);   // "Başlat" butonunun olduğu ekranı çizer
extern void draw_ui_main_desktop(void);     // Hava durumu, saat, terminal barındıran masaüstü

// ==============================================================================
// 💾 METİN MODU VE SİSTEM BELLEK TANIMLAMALARI
// ==============================================================================
uint16_t* const TEXT_VIDEO_MEMORY = (uint16_t*)0xB8000;
int text_x = 0;
int text_y = 0;

// Sistem doğrudan grafik kurulum modunda (Arama 1) başladığı için 1 olarak kilitlendi.
int is_graphics_mode = 1; 

char cmd_buffer[16];
int cmd_idx = 0;

// ==============================================================================
// 🛠️ ÇEKİRDEK İÇ METİN VE PORT GİRİŞ/ÇIKIŞ FONKSİYONLARI
// ==============================================================================

void clear_text_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        TEXT_VIDEO_MEMORY[i] = (0x0F << 8) | ' ';
    }
    text_x = 0;
    text_y = 0;
}

void print_string(const char* str) {
    if (is_graphics_mode) return; // Grafik modundaysak metin belleğine yazmayı durdur
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

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// CLI Komut Köprüsü (Doğrudan grafik modunda başladığımız için yedek mekanizmadır)
void handle_cli_command(const char* cmd) {
    if (cmd[0] == 's' && cmd[1] == 's') {
        print_string("\n[+] Sky Core OS Grafik Motoru Yukleniyor...\n");
        is_graphics_mode = 1;
        setup_init(); 
    } else {
        print_string("\nBilinmeyen Komut! Grafik moda gecmek icin 'ss' yazin.\nSkyCoreOS> ");
    }
}

// ==============================================================================
// 🗺️ UI DURUM GEÇİŞ YÖNETİCİSİ (STATE CONTROLLER)
// ==============================================================================
// Kurulum aşamalarını ve masaüstü geçişini milimetrik tetikleyen motor fonksiyon.

void refresh_system_display(void) {
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

// Kurulum sihirbazında ileri gitmeyi sağlayan çekirdek fonksiyonu
void advance_os_stage(void) {
    if (current_os_state < STATE_DESKTOP) {
        current_os_state++;
        refresh_system_display();
    }
}

// Kurulum sihirbazında geri gelmeyi sağlayan çekirdek fonksiyonu
void regress_os_stage(void) {
    if (current_os_state > STATE_WELCOME && current_os_state != STATE_DESKTOP) {
        current_os_state--;
        refresh_system_display();
    }
}

// ==============================================================================
// 🚀 MAIN KERNEL ENTRY POINT (ANA ÇEKİRDEK GİRİŞ NOKTASI)
// ==============================================================================

void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)mboot_ptr;
    (void)magic;
    
    // --------------------------------------------------------------------------
    // TERMINAL BAYPAS NOKTASI - DOĞRUDAN GRAFİKSEL KONTROL
    // --------------------------------------------------------------------------
    // Siyah ekran CLI modunu tamamen eziyoruz. 
    // Ekran kartı tamponu ilklendirilir ilklendirilmez ilk kurulum arayüzünü ateşliyoruz.
    setup_init(); 
    
    // İlk ekran durumunu (STATE_WELCOME) grafik tamponuna çiziyoruz
    current_os_state = STATE_WELCOME;
    refresh_system_display();

    // Komut tamponu temizliği ve bellek güvenliği
    for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;

    // ==========================================================================
    // ⌨️ ULTRA POLLING KLAVYE MOTORU & SCANTABLE FİLTRELEME DÖNGÜSÜ
    // ==========================================================================
    while (1) {
        // PS/2 Klavye Kontrolcüsü Port Taraması (Status Register bit 0 kontrolü)
        if (inb(0x64) & 1) { 
            uint8_t scancode = inb(0x60);
            
            if (is_graphics_mode) {
                // 1. Genel Girdi Kontrolü: Alınan donanımsal girdiyi alt sisteme (setup.c) ilet
                setup_handle_input(scancode); 
                
                // 2. Özel Çekirdek Kısayolları (Make Code Filtreleme - Tuştan el çekilmediyse)
                if (!(scancode & 0x80)) {
                    switch (scancode) {
                        case 0x1C: // ENTER Tuşu (Kurulumda İleri Git / Masaüstüne Geç)
                            advance_os_stage();
                            break;
                            
                        case 0x0E: // BACKSPACE Tuşu (Kurulumda Bir Önceki Aşamaya Dön)
                            regress_os_stage();
                            break;
                            
                        default:
                            break;
                    }
                }
            } else {
                // Siyah Terminal Modu Aktifse (Yedek Güvenlik Katmanı)
                if (!(scancode & 0x80)) {
                    if (scancode == 0x1F) { // 'S' Tuşu
                        if (cmd_idx < 14) { 
                            cmd_buffer[cmd_idx++] = 's'; 
                            print_string("s"); 
                        }
                    } else if (scancode == 0x1C) { // ENTER Tuşu
                        cmd_buffer[cmd_idx] = '\0';
                        handle_cli_command(cmd_buffer);
                        
                        // Buffer sıfırlama
                        for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
                        cmd_idx = 0;
                    }
                }
            }
        }
        
        // İşlemcinin (CPU) %100 yük altında kavrulmasını engellemek için 
        // assembly seviyesinde pipeline duraklatması emrediyoruz.
        __asm__ volatile("pause");
    }
}
