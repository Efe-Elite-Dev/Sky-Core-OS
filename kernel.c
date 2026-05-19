#include "globals.h"
#include "gui.h"

// Varsayılan LFB adresi (GRUB'dan gelen gerçek adresle güncellenecek)
uint32_t* gfx_framebuffer = (uint32_t*)0xE0000000; 

SystemState current_state = STATE_WELCOME;
bool ai_hud_visible = false;
SetupData os_setup_data;

void kernel_main(uint32_t magic, uint32_t* mbi) {
    // GRUB üzerinden başarıyla boot edildiyse ve grafik bilgisi mevcutsa
    if (magic == 0x2BADB002 && mbi != NULL) {
        // Multiboot yapısının 18. elemanı bize ekran kartının GERÇEK adresini verir
        uint32_t real_fb = mbi[18]; 
        if (real_fb != 0) {
            gfx_framebuffer = (uint32_t*)real_fb;
        }
    }

    init_vga();
    os_setup_data.wifi_connected = false;

    while (true) {
        switch (current_state) {
            case STATE_WELCOME:
                draw_setup_welcome();
                break;
            case STATE_LOCATION:
                draw_setup_location();
                break;
            case STATE_COMPLETING:
                draw_setup_completing();
                break;
            case STATE_DESKTOP:
                draw_main_desktop();
                break;
        }

        if (ai_hud_visible) {
            draw_ai_subsystem_hud();
        }
    }
}
