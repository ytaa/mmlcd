#ifndef LIBMMLCD_IPC_H
#define LIBMMLCD_IPC_H

#include <stdint.h>

#define LIBLCD_IPC_PRINT_STRING_SIZE 17u /* including null terminator */
#define LIBLCD_IPC_SOCKET_PATH "/tmp/mmlcd_daemon.sock"

typedef enum {
    liblcd_ipc_cmd_print = 0,
    liblcd_ipc_cmd_clear,
    liblcd_ipc_cmd_backlight,
    liblcd_ipc_cmd_count,
    liblcd_ipc_cmd_invalid = liblcd_ipc_cmd_count
} liblcd_ipc_cmd;

struct liblcd_ipc_print_params_s {
    char str[LIBLCD_IPC_PRINT_STRING_SIZE];
    uint8_t line;
    uint8_t pos;
}__attribute__((packed));

typedef struct liblcd_ipc_print_params_s liblcd_ipc_print_params;

struct liblcd_ipc_print_req_s {
    liblcd_ipc_cmd cmd;
    liblcd_ipc_print_params params;
}__attribute__((packed));

typedef struct liblcd_ipc_print_req_s liblcd_ipc_print_req;

struct liblcd_ipc_backlight_params_s {
    uint8_t enable;
}__attribute__((packed));

typedef struct liblcd_ipc_backlight_params_s liblcd_ipc_backlight_params;

struct liblcd_ipc_backlight_req_s {
    liblcd_ipc_cmd cmd;
    liblcd_ipc_backlight_params params;
}__attribute__((packed));

typedef struct liblcd_ipc_backlight_req_s liblcd_ipc_backlight_req;

#endif // LIBLCD_IPC_H