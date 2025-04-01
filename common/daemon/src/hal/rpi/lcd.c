#include <unistd.h>
#include <string.h>
#include <pigpio.h>

#include "lcd.h"

/* config */
#define LCD_ADDRESS 0x27u
#define LCD_I2C_BUS 1u 

/* timings */
#define LCD_DELAY_INIT 200000 /* us */
#define LCD_DELAY_CMD 100 /* us */
#define LCD_DELAY_STROBE 500 /* us */

/* bits */
#define LCD_BIT_ENABLE 0b00000100u /* Enable bit */
#define LCD_BIT_RW 0b00000010u /* Read/Write bit */
#define LCD_BIT_RS 0b00000001u /* Register select bit */

static bool is_pigpio_initialized = false;

/* wrapper for pigpio function adding delay after i2c write */
static int i2cWriteByteDelayed(int handle, uint8_t byte){
    int ret = i2cWriteByte(handle, byte);
    usleep(LCD_DELAY_CMD);
    return ret;
}

/* initializes objects and lcd */
int lcd_init(void){
    if(!is_pigpio_initialized){
        /* initialize pigpio lib */
        int ret = gpioInitialise();

        if(0 > ret){
            /* failed to initialize pigpio */
            return ret;
        }

        is_pigpio_initialized = true;
    }

    return i2cOpen(LCD_I2C_BUS, LCD_ADDRESS, 0u /* flags */);
}

void lcd_deinit(int handle){
    if(0 <= handle){
        lcd_backlight(handle, 0);
    }

    gpioTerminate();
    is_pigpio_initialized = false;
}

int lcd_setup(int handle){
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    /* write initial magic bytes */
    lcd_write(handle, LCD_CMD_MAGIC_INIT_1);
    lcd_write(handle, LCD_CMD_MAGIC_INIT_1);
    lcd_write(handle, LCD_CMD_MAGIC_INIT_1);
    lcd_write(handle, LCD_CMD_MAGIC_INIT_2);

    /* initial setup */
    lcd_write(handle, LCD_CMD_FUNCTIONSET | LCD_CMD_2LINE | LCD_CMD_5x8DOTS | LCD_CMD_4BITMODE);
    lcd_write(handle, LCD_CMD_DISPLAYCONTROL | LCD_CMD_DISPLAYON);
    lcd_write(handle, LCD_CMD_CLEARDISPLAY);
    lcd_write(handle, LCD_CMD_ENTRYMODESET | LCD_CMD_ENTRYLEFT);
    
    usleep(LCD_DELAY_INIT);

    return 0;
}

/* clocks EN to latch command */
int lcd_strobe(int handle, uint8_t data){
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    i2cWriteByteDelayed(handle, data | LCD_BIT_ENABLE | LCD_CMD_BACKLIGHT);
    usleep(LCD_DELAY_STROBE);

    i2cWriteByteDelayed(handle, ((data & ~LCD_BIT_ENABLE) | LCD_CMD_BACKLIGHT));
    usleep(LCD_DELAY_CMD);

    return 0;
}

int lcd_write_four_bits(int handle, uint8_t data){
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    i2cWriteByteDelayed(handle, data | LCD_CMD_BACKLIGHT);
    lcd_strobe(handle, data);

    return 0;
}

/* write a command to lcd */
int lcd_write(int handle, uint8_t cmd){
    return lcd_write_mode(handle, cmd, 0u);
}

int lcd_write_mode(int handle, uint8_t cmd, uint8_t mode){
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    lcd_write_four_bits(handle, mode | (cmd & 0xF0));
    lcd_write_four_bits(handle, mode | ((cmd << 4) & 0xF0));

    return 0;
}

int lcd_display_string(int handle, const char * const string){
    return lcd_display_string_pos(handle, string, 1u, 0u);
}

int lcd_display_string_pos(int handle, const char * const string, uint8_t line, uint8_t pos){
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    uint8_t pos_new = pos;

    switch(line){
        case 1u:{
            break;
        }
        case 2u:{
            pos_new += 0x40u;
            break;
        }
        case 3u:{
            pos_new += 0x14u;
            break;
        }
        case 4u:{
            pos_new += 0x54u;
            break;
        }
        default:{
            break;
        }
    }

    lcd_write(handle, 0x80u + pos_new);

    uint32_t len = strlen(string);
    for(uint32_t char_idx = 0u; char_idx < len; ++char_idx){
        lcd_write_mode(handle, string[char_idx], LCD_BIT_RS);
    }

    return 0;
}

/* clear lcd and set to home */
int lcd_clear(int handle) {
    if(0 > handle){
        /* invalid handle */
        return handle;
    }

    lcd_write(handle, LCD_CMD_CLEARDISPLAY);
    lcd_write(handle, LCD_CMD_RETURNHOME);

    return 0;
}

/* backlight on/off */
int lcd_backlight(int handle, bool state){
    return i2cWriteByteDelayed(handle, state ? LCD_CMD_BACKLIGHT : LCD_CMD_NOBACKLIGHT);
}

/* add custom characters (0 - 7) 
def lcd_load_custom_chars(self, fontdata):
    self.lcd_write(0x40);
    for char in fontdata:
        for line in char:
        self.lcd_write_char(line) 
*/