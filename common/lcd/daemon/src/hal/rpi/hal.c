#include "hal.h"
#include "rpi/lcd.h"

int hal_lcd_init(void){
    return lcd_init();
}

void hal_lcd_deinit(int handle){
    lcd_deinit(handle);
}

int hal_lcd_setup(int handle){
    return lcd_setup(handle);
}

int hal_lcd_clear(int handle){
    return lcd_clear(handle);
}

int hal_lcd_display_string_pos(int handle, const char * const string, uint8_t line, uint8_t pos){
    return lcd_display_string_pos(handle, string, line, pos);
}

int hal_lcd_backlight(int handle, bool state){
    return lcd_backlight(handle, state);
}