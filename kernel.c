#include "globals.h"
#include "gui.h"

SystemState current_state = STATE_WELCOME;
SetupData os_setup_data;

void kernel_main() {
    init_vga();
    
    // Varsayılan Kurulum Verileri
    os_setup_data.wifi_connected = false;

    // Ana Grafik ve Durum Döngüsü
    while (true) {
        switch (current_state) {
            case STATE_WELCOME:
                draw_setup_welcome();
                // Simüle edilmiş tıklama veya klavye ile geçiş:
                // current_state = STATE_LOCATION;
                break;
                
            case STATE_LOCATION:
                draw_setup_location();
                // current_state = STATE_COMPLETING;
                break;
                
            case STATE_COMPLETING:
                draw_setup_completing();
                // current_state = STATE_DESKTOP;
                break;
                
            case STATE_DESKTOP:
                draw_main_desktop();
                break;
        }

        // Eğer ALT+SPACE aktifse, mevcut ekranın üzerine AI panelini bindir
        if (ai_hud_visible) {
            draw_ai_subsystem_hud();
        }
    }
}
