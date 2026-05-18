// ==============================================================================
//  WIND OS / SKY CORE OS - ÇEKİRDEK HATA VE ÇÖZÜM YÖNETİMİ (kerror.c)
// ==============================================================================
#include <stdint.h>

// Hata Kodları Tanımlamaları
#define ERR_GRAPHICS_FAIL       0x01  // Grafik modu açılamadı veya LFB kilitlendi
#define ERR_PAGE_FAULT          0x02  // Olmayan bellek adresine sızma (Taşma)
#define ERR_IDT_CRASH           0x03  // Kesme tablosu veya Sürücü çökmesi
#define ERR_SUBSYSTEM_FAIL      0x04  // Yapay zeka veya EXE subsistem hatası

extern uint32_t* GRAPHICS_FRAMEBUFFER;
extern void force_graphics_hardware(void);

// Ekranı acil durum rengine (Örn: Parlak Kırmızı veya Lacivert) boyayan fonksiyon
void fill_emergency_screen(uint32_t color) {
    for (int i = 0; i < 1024 * 768; i++) {
        GRAPHICS_FRAMEBUFFER[i] = color;
    }
}

// 🔥 TÜM OLASI HATALARI YAKALAYIP ÇÖZÜMÜNÜ DİKTE EDEN ANA MOTOR
void kpanic(uint8_t error_code, const char* message) {
    // Ne olur ne olmaz donanımı güvenli moda zorla
    force_graphics_hardware();
    
    // Ekranı "Kritik Hata" kırmızısına boyayalım (0xFF0000)
    fill_emergency_screen(0xFF0000);

    // Ekrana hata detaylarını ve çözümlerini basma mantığı
    // (Burada senin gui veya screen piksellerini kullanarak metin bastığını varsayıyoruz)
    
    switch(error_code) {
        case ERR_GRAPHICS_FAIL:
            // ÇÖZÜMÜ: VirtualBox VBoxVGA moduna alınmalı ve video belleği artırılmalı!
            break;
            
        case ERR_PAGE_FAULT:
            // ÇÖZÜMÜ: C kodunda dizi sınırları veya pointer adresleri kontrol edilmeli!
            break;
            
        case ERR_IDT_CRASH:
            // ÇÖZÜMÜ: Assembly kesme (IRQ) el sıkışması ve push/pop dengesi düzeltilmeli!
            break;
            
        case ERR_SUBSYSTEM_FAIL:
            // ÇÖZÜMÜ: win_subsystem veya ai_subsystem içindeki null pointer'lar temizlenmeli!
            break;
            
        default:
            // Bilinmeyen Kritik Çökme!
            break;
    }

    // İşlemciyi sonsuza kadar kilitle ki donanım daha fazla zarar görmesin
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}
