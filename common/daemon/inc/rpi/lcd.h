#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

#define LCD_CMD_CLEARDISPLAY 0x01u
#define LCD_CMD_RETURNHOME 0x02u
#define LCD_CMD_ENTRYMODESET 0x04u
#define LCD_CMD_DISPLAYCONTROL 0x08u
#define LCD_CMD_CURSORSHIFT 0x10u
#define LCD_CMD_FUNCTIONSET 0x20u
#define LCD_CMD_SETCGRAMADDR 0x40u
#define LCD_CMD_SETDDRAMADDR 0x80u

/* magic */
#define LCD_CMD_MAGIC_INIT_1 0x03u
#define LCD_CMD_MAGIC_INIT_2 0x02u

/* flags for display entry mode */
#define LCD_CMD_ENTRYRIGHT 0x00u
#define LCD_CMD_ENTRYLEFT 0x02u
#define LCD_CMD_ENTRYSHIFTINCREMENT 0x01u
#define LCD_CMD_ENTRYSHIFTDECREMENT 0x00u

/* flags for display on/off control */
#define LCD_CMD_DISPLAYON 0x04u
#define LCD_CMD_DISPLAYOFF 0x00u
#define LCD_CMD_CURSORON 0x02u
#define LCD_CMD_CURSOROFF 0x00u
#define LCD_CMD_BLINKON 0x01u
#define LCD_CMD_BLINKOFF 0x00u

/* flags for display/cursor shift */
#define LCD_CMD_DISPLAYMOVE 0x08u
#define LCD_CMD_CURSORMOVE 0x00u
#define LCD_CMD_MOVERIGHT 0x04u
#define LCD_CMD_MOVELEFT 0x00u

/* flags for function set */
#define LCD_CMD_8BITMODE 0x10u
#define LCD_CMD_4BITMODE 0x00u
#define LCD_CMD_2LINE 0x08u
#define LCD_CMD_1LINE 0x00u
#define LCD_CMD_5x10DOTS 0x04u
#define LCD_CMD_5x8DOTS 0x00u

/* flags for backlight control */
#define LCD_CMD_BACKLIGHT 0x08u
#define LCD_CMD_NOBACKLIGHT 0x00u

int lcd_init(void);

void lcd_deinit(int handle);

int lcd_setup(int handle);

/* clocks EN to latch command */
int lcd_strobe(int handle, uint8_t data);

/* write a command to lcd */
int lcd_write(int handle, uint8_t cmd);

int lcd_write_mode(int handle, uint8_t cmd, uint8_t mode);
   
int lcd_display_string(int handle, const char * const string);

int lcd_display_string_pos(int handle, const char * const string, uint8_t line, uint8_t pos);

int lcd_clear(int handle);

/* backligt on/off */
int lcd_backlight(int handle, bool state);

#endif /* LCD_DEV_H */