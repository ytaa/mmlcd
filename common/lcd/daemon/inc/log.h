#ifndef LOG_H
#define LOG_H

#include <syslog.h>

/* --- Macros and constants ---------------------------- */

/* --- Type definitions -------------------------------- */

/* --- Extern variables -------------------------------- */

/* --- Public functions declarations ------------------- */

void slog(int priority, const char *format, ...);

#endif