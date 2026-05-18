#ifndef SETUP_H
#define SETUP_H

#include <stdint.h>

typedef enum {
    STAGE_WIND_COUNTRY = 0,    
    STAGE_WIND_KEYBOARD,       
    STAGE_WIND_NETWORK,        
    STAGE_WIND_HOSTNAME,       
    STAGE_WIND_CUSTOMIZE,      
    STAGE_WIND_COMPLETE
} SetupStage;

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

#define COLOR_WIND_BG        16  
#define COLOR_WIND_CARD      17  
#define COLOR_WIND_TEXT      18  
#define COLOR_WIND_PRIMARY   19  
#define COLOR_WIND_SWIRL     20  
#define COLOR_WIND_3D_LIGHT  21  
#define COLOR_WIND_3D_DARK   22  

void setup_init(void);
void setup_render(void);
void setup_handle_input(uint8_t scancode);

#endif
