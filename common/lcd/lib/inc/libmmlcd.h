#ifndef LIBMMLCD_H
#define LIBMMLCD_H

#include <stdint.h>
#include <stdbool.h>
#include "libmmlcd_ipc.h"

/* --- Macros and constants ---------------------------- */

/* --- Type definitions -------------------------------- */

typedef void (*liblcd_btn_event_callback)(liblcd_ipc_btn_event);

/* --- Extern variables -------------------------------- */

/* --- Public functions declarations ------------------- */

int liblcd_init(void);

void liblcd_deinit(void);

int liblcd_display_string_pos(const char * const string, uint8_t line, uint8_t pos);

int liblcd_clear();

/* backligt on/off */
int liblcd_backlight(bool enable);

int liblcd_get_btn_state(liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state);

int liblcd_register_btn_event_callback(liblcd_btn_event_callback cbk);

#endif /* LIBLCD_H */