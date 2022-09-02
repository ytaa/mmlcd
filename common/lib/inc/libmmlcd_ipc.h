#ifndef LIBMMLCD_IPC_H
#define LIBMMLCD_IPC_H

/* --- Macros and constants ---------------------------- */

#define LIBLCD_IPC_PRINT_STRING_SIZE 17u /* including null terminator */
#define LIBLCD_IPC_SOCKET_PATH "/tmp/mmlcd_daemon.sock"

/* --- Type definitions -------------------------------- */

typedef enum {
    liblcd_ipc_cmd_print = 0,
    liblcd_ipc_cmd_clear,
    liblcd_ipc_cmd_backlight,
    liblcd_ipc_cmd_get_btn_state,
    liblcd_ipc_cmd_count,
    liblcd_ipc_cmd_invalid = liblcd_ipc_cmd_count
} liblcd_ipc_cmd;

struct liblcd_ipc_print_params_s {
    char str[LIBLCD_IPC_PRINT_STRING_SIZE];
    unsigned char line;
    unsigned char pos;
}__attribute__((packed));

typedef struct liblcd_ipc_print_params_s liblcd_ipc_print_params;

struct liblcd_ipc_print_req_s {
    liblcd_ipc_cmd cmd;
    liblcd_ipc_print_params params;
}__attribute__((packed));

typedef struct liblcd_ipc_print_req_s liblcd_ipc_print_req;

struct liblcd_ipc_backlight_params_s {
    unsigned char enable;
}__attribute__((packed));

typedef struct liblcd_ipc_backlight_params_s liblcd_ipc_backlight_params;

struct liblcd_ipc_backlight_req_s {
    liblcd_ipc_cmd cmd;
    liblcd_ipc_backlight_params params;
}__attribute__((packed));

typedef struct liblcd_ipc_backlight_req_s liblcd_ipc_backlight_req;

enum liblcd_ipc_btn_state_e {
    liblcd_ipc_btn_released = 0,
    liblcd_ipc_btn_pressed,
    liblcd_ipc_btn_state_count,
    liblcd_ipc_btn_state_invalid = liblcd_ipc_btn_state_count
};

typedef enum liblcd_ipc_btn_state_e liblcd_ipc_btn_state;

enum liblcd_ipc_btn_idx_e {
    liblcd_ipc_btn_first = 0,
    liblcd_ipc_btn_1 = liblcd_ipc_btn_first,
    liblcd_ipc_btn_2,
    liblcd_ipc_btn_3,
    liblcd_ipc_btn_count,
    liblcd_ipc_btn_idx_invlid = liblcd_ipc_btn_count
};

typedef enum liblcd_ipc_btn_idx_e liblcd_ipc_btn_idx;

struct liblcd_ipc_btn_event_s {
    liblcd_ipc_btn_state state;
    liblcd_ipc_btn_idx idx;
};

typedef struct liblcd_ipc_btn_event_s liblcd_ipc_btn_event;

struct liblcd_ipc_get_btn_state_params_s {
    liblcd_ipc_btn_idx idx;
}__attribute__((packed));

typedef struct liblcd_ipc_get_btn_state_params_s liblcd_ipc_get_btn_state_params;

struct liblcd_ipc_get_btn_state_req_s {
    liblcd_ipc_cmd cmd;
    liblcd_ipc_get_btn_state_params params;
}__attribute__((packed));

typedef struct liblcd_ipc_get_btn_state_req_s liblcd_ipc_get_btn_state_req;

/* --- Extern variables -------------------------------- */

/* --- Public functions declarations ------------------- */

#endif // LIBLCD_IPC_H