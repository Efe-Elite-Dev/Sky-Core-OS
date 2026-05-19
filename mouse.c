#include "mouse.h"

// Mouse paket verileri
static int mouse_x = 400, mouse_y = 300; 

void mouse_handler() {
    uint8_t status = inb(0x64);
    if (!(status & 0x01)) return; // Veri yoksa çık

    int8_t mouse_byte = inb(0x60);
    // Burada 3 baytlık mouse paketini ayrıştırıp
    // mouse_x ve mouse_y değerlerini güncellemen gerekiyor.
    
    // Örnek: mouse_x += byte2;
    // Örnek: mouse_y -= byte3; 
    
    update_cursor_position(mouse_x, mouse_y);
}
