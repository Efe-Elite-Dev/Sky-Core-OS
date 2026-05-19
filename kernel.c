#include "sky_core.h"

SystemState current_state = BOOT;

void kernel_main() {
    init_gui();
    init_mouse();

    while(1) {
        switch(current_state) {
            case BOOT:
                // Boot animasyonu
                current_state = SETUP;
                break;
            case SETUP:
                draw_setup_screen(0, NULL);
                break;
            case DESKTOP:
                draw_desktop();
                break;
        }
    }
}
