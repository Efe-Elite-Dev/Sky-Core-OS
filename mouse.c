#include "wind_subsystem.h"
#include "mouse.h"  /* Kendi başlık dosyasını da ekleyerek prototipleri kilitliyoruz */

static int mouse_x = 400;
static int mouse_y = 300;
static int mouse_click_stress = 0;

/* Fare Sürücüsü Ham Veri Toplama Modülü */
void handle_mouse_polling(void) {
    // PS/2 Fare portundan (0x64 durum, 0x60 veri) veri okuma donanım sorgusu
    uint8_t status = inb(0x64);
    
    if (status & 0x01) {
        uint8_t data = inb(0x60);
        // Fare hareket yönü ve ivme verisine göre stres analizi girdisi oluştur
        if (data > 0) {
            mouse_click_stress++;
        }
    }

    // Çekirdeğin ekran taşması yapıp çökmesini engelleyen güvenlik sınırları
    if (mouse_x > 800) mouse_x = 799;
    if (mouse_y > 600) mouse_y = 599;
}

/* Yapay Zeka İçin Fare Hareket ve Stres Analiz Ajanı */
int ai_mouse_analyze_stress(void) {
    int current_stress = mouse_click_stress % 100;
    // Okuma yapıldıktan sonra stres birikimini yavaşça sönümle
    if (mouse_click_stress > 0) {
        mouse_click_stress--;
    }
    return current_stress; // %0-100 arası anlık stres değerini döner
}

void init_mouse(void) {
    mouse_x = 400;
    mouse_y = 300;
    mouse_click_stress = 0;
}
