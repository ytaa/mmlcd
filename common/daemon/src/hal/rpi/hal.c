#include "hal.h"
#include "rpi/lcd.h"

#include <pigpio.h> 


/* --- Macros ------------------------------------------ */

#define PIN_BUTTON1 17
#define PIN_BUTTON2 27
#define PIN_BUTTON3 22

/* --- Private functions declarations ------------------ */

static int hal_lcd_init(void);
static void hal_lcd_deinit(void);
static int hal_btn_init(void);
static void hal_btn_deinit(void);

/* --- Global variables -------------------------------- */

static int g_lcd_dev_handle = -1;

static int g_btn_pins[liblcd_ipc_btn_count] = { 
    PIN_BUTTON1,
    PIN_BUTTON2,
    PIN_BUTTON3
};

/* --- Public functions definitions -------------------- */

int hal_init(void){
    if(hal_lcd_init()){
        return -1;
    }

    if(hal_btn_init()){
        return -1;
    }

    return 0;
}

void hal_deinit(void){
    hal_lcd_deinit();
    hal_btn_deinit();
}

int hal_lcd_clear(liblcd_ipc_mmlcd_addr addr){
    (void) addr; /* address not used on the device */
    return lcd_clear(g_lcd_dev_handle);
}

int hal_lcd_display_string_pos(liblcd_ipc_mmlcd_addr addr, const char * const string, uint8_t line, uint8_t pos){
    (void) addr; /* address not used on the device */
    return lcd_display_string_pos(g_lcd_dev_handle, string, line, pos);
}

int hal_lcd_backlight(liblcd_ipc_mmlcd_addr addr, bool state){
    (void) addr; /* address not used on the device */
    return lcd_backlight(g_lcd_dev_handle, state);
}

int hal_btn_get_state(liblcd_ipc_mmlcd_addr addr, liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state){
    (void) addr; /* address not used on the device */

    if(!(state) || liblcd_ipc_btn_idx_invlid <= idx){
        return -1;
    }

    *state = gpioRead(g_btn_pins[idx]) ? liblcd_ipc_btn_released : liblcd_ipc_btn_pressed;

    return 0;
}

/* --- Private functions definitions -------------------- */


static int hal_lcd_init(void){
    g_lcd_dev_handle = lcd_init();
    if(0 > g_lcd_dev_handle){
        return g_lcd_dev_handle;
    }

    return lcd_setup(g_lcd_dev_handle);
}

static void hal_lcd_deinit(void){
    lcd_deinit(g_lcd_dev_handle);
    g_lcd_dev_handle = -1;
}

static int hal_btn_init(void){
    int ret = gpioInitialise();
    if(0 > ret){
        return ret;
    }

    /* setup pushbuttons pins */
    for(unsigned int btn_idx = liblcd_ipc_btn_first; btn_idx < liblcd_ipc_btn_count; ++btn_idx){
        gpioSetMode(g_btn_pins[btn_idx], PI_INPUT);
        gpioSetPullUpDown(g_btn_pins[btn_idx], PI_PUD_UP);
    }

    return 0;
}

static void hal_btn_deinit(void){
    gpioTerminate();
}