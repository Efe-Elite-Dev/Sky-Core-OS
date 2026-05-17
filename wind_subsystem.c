#include "wind_subsystem.h"
#include "ai_subsystem.h"

/* Rüzgar Alt Sistemi Global Durum Değişkenleri */
static int system_initialized = 0;
static uint32_t system_uptime_ticks = 0;

/**
 * @brief Rüzgar Alt Sistemini (Wind Subsystem) ilkfırlatan ana motor.
 */
void init_wind_subsystem(void) {
    system_initialized = 1;
    system_uptime_ticks = 0;
}

/**
 * @brief Alt sistemin kalbini vuran, her PIT kesmesinde tetiklenen rutin döngü.
 */
void wind_subsystem_tick(void) {
    if (!system_initialized) return;
    system_uptime_ticks++;
}

/**
 * @brief DÜZELTME: wind_subsystem.h içindeki void prototipi ile %100 eşitlendi!
 * Çakışma yaratan eski 'int' gövde tamamen kaldırılarak çekirdekle mühürlendi.
 */
void ai_core_predict_scheduler(int predicted_load, int anomaly_flag, int policy) {
    // Derleyicinin "unused parameter" hata korumalarını devreye alıyoruz
    (void)predicted_load;
    (void)anomaly_flag;
    (void)policy;
    
    // Bu fonksiyonun asıl işlevsel gövdesi artık ai_subsystem.c içinde yaşıyor.
    // Burada prototip senkronizasyonunu korumak ve gerekirse tetikleme yapmak için boş tutuluyor.
}

/**
 * @brief Rüzgar alt sisteminin mevcut çalışma zamanı tik değerini döner.
 */
uint32_t get_wind_subsystem_uptime(void) {
    return system_uptime_ticks;
}
