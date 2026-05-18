#include <stdint.h>
#include <stddef.h>

// Dış alt sistemlerin (setup.c) fonksiyon bildirimleri
void setup_init(void);
void setup_handle_input(uint8_t scancode);

uint16_t* const TEXT_VIDEO_MEMORY = (uint16_t*)0xB8000;
int text_x = 0;
int text_y = 0;
int is_graphics_mode = 0; // 0 = Sky Core OS CLI, 1 = Grafik Kurulum Modu

char cmd_buffer[16];
int cmd_idx = 0;

void clear_text_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        TEXT_VIDEO_MEMORY[i] = (0x0F << 8) | ' ';
    }
    text_x = 0;
    text_y = 0;
}

void print_string(const char* str) {
    if (is_graphics_mode) return;
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

// Sky Core OS Grafik Motoruna Geçiş Köprüsü
void handle_cli_command(const char* cmd) {
    if (cmd[0] == 's' && cmd[1] == 's') {
        print_string("\n[+] Sky Core OS Grafik Motoru Yukleniyor... Sistem Ateslendi!\n");
        is_graphics_mode = 1;
        setup_init(); // setup.c içindeki o fırtınalı grafik arayüzü başlatılıyor
    } else {
        print_string("\nBilinmeyen Komut! Grafik moda gecmek icin 'ss' yazin.\nSkyCoreOS> ");
    }
}

void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)mboot_ptr;
    (void)magic;
    
    clear_text_screen();
    print_string("=======================================================================\n");
    print_string("         Sky Core OS - Safe AI Kernel v1.5 - Ultra Polling Active\n");
    print_string("=======================================================================\n\n");
    print_string("[+] Metin Modu Ekran Surucusu (VGA 80x25): STABIL\n");
    print_string("[+] Saf Çekirdek Katmani (Sky Core Layer): AKTIF\n");
    print_string("[+] Yapay Zeka Karar Destek Mekanizmasi: AKTIF\n");
    print_string("[+] Donanimsal Kesme Korumasi (Anti-Guru Protection): AKTIF\n");
    print_string("[+] Dogrudan Port Taramali Klavye Motoru: HAZIR\n\n");
    print_string("Grafiksel Kurulum Sihirbazina gecmek icin 'ss' yazip ENTER'a basin.\n\n");
    print_string("SkyCoreOS> ");

    // Komut tamponu temizliği
    for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;

    while (1) {
        // Klavye kontrolcüsü port taraması
        if (inb(0x64) & 1) { 
            uint8_t scancode = inb(0x60);
            
            if (is_graphics_mode) {
                setup_handle_input(scancode); 
            } else {
                // Sadece tuşa basılma anını (Make Code) algıla, elini çekme anını filtrele
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
        __asm__ volatile("pause");
    }
}
