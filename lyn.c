#include <stdint.h>
#include <stddef.h>

/* =========================================================
 * EKSİK SYMBOL FIXLERI
 * ========================================================= */

void force_graphics_hardware(void)
{
}

int ai_hud_visible = 0;

void screen_init(void) {}
void setup_init(void) {}

void setup_handle_input(uint8_t sc)
{
    (void)sc;
}

void keyboard_init(void) {}
void wind_subsystem_init(void) {}
void ai_subsystem_init(void) {}
