#ifndef LIBMMLCD_H
#define LIBMMLCD_H

#include <stdint.h>
#include <stdbool.h>

int liblcd_init(void);

void liblcd_deinit(void);

int liblcd_display_string_pos(const char * const string, uint8_t line, uint8_t pos);

int liblcd_clear();

/* backligt on/off */
int liblcd_backlight(bool enable);

#endif /* LIBLCD_H */