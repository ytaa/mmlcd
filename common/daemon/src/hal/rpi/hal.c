#include "hal.h"
#include "rpi/lcd.h"

#include <pigpio.h> 

#define PIN_BUTTON1 17
#define PIN_BUTTON2 27
#define PIN_BUTTON3 22

static int btn_pins[liblcd_ipc_btn_count] = { 
    PIN_BUTTON1,
    PIN_BUTTON2,
    PIN_BUTTON3
};

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

int hal_btn_init(void){
    int ret = gpioInitialise();
    if(0 > ret){
        return ret;
    }

    /* setup pushbuttons pins */
    for(unsigned int btn_idx = liblcd_ipc_btn_first; btn_idx < liblcd_ipc_btn_count; ++btn_idx){
        gpioSetMode(btn_pins[btn_idx], PI_INPUT);
        gpioSetPullUpDown(btn_pins[btn_idx], PI_PUD_UP);
    }

    return 0;
}

void hal_btn_deinit(void){
    gpioTerminate();
}

int hal_btn_get_state(liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state){
    if(!(state) || liblcd_ipc_btn_idx_invlid <= idx){
        return -1;
    }

    *state = gpioRead(btn_pins[idx]) ? liblcd_ipc_btn_released : liblcd_ipc_btn_pressed;

    return 0;
}