#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stdint.h>

#include "libmmlcd_ipc.h"

int hal_lcd_init(void);

void hal_lcd_deinit(int handle);

int hal_lcd_setup(int handle);

int hal_lcd_clear(int handle);

int hal_lcd_display_string_pos(int handle, const char * const string, uint8_t line, uint8_t pos);

int hal_lcd_backlight(int handle, bool state);

int hal_btn_init(void);

void hal_btn_deinit(void);

int hal_btn_get_state(liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state);

#endif