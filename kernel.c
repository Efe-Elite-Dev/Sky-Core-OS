#include <stdint.h>
#include <stddef.h>

// Dış alt sistemlerin (setup.c) fonksiyon bildirimleri
void setup_init(void);
void setup_handle_input(uint8_t scancode);

uint16_t* const TEXT_VIDEO_MEMORY = (uint16_t*)0xB8000;
int text_x = 0;
int text_y = 0;

// ARTIK VARSAYILAN OLARAK 1: Sistem doğrudan grafik kurulum modunda başlıyor
int is_graphics_mode = 1; 

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

// Grafik modunda başladığımız için bu köprü normal şartlarda baypas edilecek
void handle_cli_command(const char* cmd) {
    if (cmd[0] == 's' && cmd[1] == 's') {
        print_string("\n[+] Sky Core OS Grafik Motoru Yukleniyor... Sistem Ateslendi!\n");
        is_graphics_mode = 1;
        setup_init(); 
    } else {
        print_string("\nBilinmeyen Komut! Grafik moda gecmek icin 'ss' yazin.\nSkyCoreOS> ");
    }
}

void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)mboot_ptr;
    (void)magic;
    
    // ------------------------------------------------------------------
    // TERMINALİ ES GEÇİP DOĞRUDAN GRAFİK MOTORUNU ATEŞLEME NOKTASI
    // ------------------------------------------------------------------
    // Metin modu yazılarını ve ekran temizlemeyi tamamen pas geçiyoruz.
    // Sistem boot edildiği an senin o fırtınalı grafik arayüzünü başlatıyoruz.
    setup_init(); 

    // Komut tamponu temizliği (Güvenlik için sıfırlayalım)
    for(int i = 0; i < 16; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;

    while (1) {
        // Klavye kontrolcüsü port taraması (Polling) kesintisiz aktif
        if (inb(0x64) & 1) { 
            uint8_t scancode = inb(0x60);
            
            // Sistem grafik modunda başladığı için doğrudan girdileri burası yakalayacak
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
