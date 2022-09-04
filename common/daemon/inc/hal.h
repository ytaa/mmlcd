#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stdint.h>

#include "libmmlcd_ipc.h"

int hal_init(void);

void hal_deinit(void);

int hal_lcd_clear(liblcd_ipc_mmlcd_addr addr);

int hal_lcd_display_string_pos(liblcd_ipc_mmlcd_addr addr, const char * const string, uint8_t line, uint8_t pos);

int hal_lcd_backlight(liblcd_ipc_mmlcd_addr addr, bool state);

int hal_btn_get_state(liblcd_ipc_mmlcd_addr addr, liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state);

#endif