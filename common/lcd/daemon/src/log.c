#include "log.h"

#include <stdarg.h>

/* --- Macros ------------------------------------------ */

/* --- Private functions declarations ------------------ */

/* --- Global variables -------------------------------- */

static int g_log_level = LOG_INFO;

/* --- Public functions definitions -------------------- */

void slog(int priority, const char *format, ...){
    if(priority <= g_log_level){
        va_list argptr;
        va_start(argptr, format);
        vsyslog(priority, format, argptr);
        va_end(argptr);
    }
}