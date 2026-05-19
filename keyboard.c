#include "globals.h"
#include "io.h"
#include "gui.h"

#define KEYBOARD_DATA_PORT 0x60

static bool alt_pressed = false;

void init_keyboard() {
    (void)inb(KEYBOARD_DATA_PORT);
}

void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0x38) {
        alt_pressed = true;
    } else if (scancode == 0xB8) {
        alt_pressed = false;
    }

    if (scancode == 0x39 && alt_pressed) {
        ai_hud_visible = !ai_hud_visible; 
    }
}
