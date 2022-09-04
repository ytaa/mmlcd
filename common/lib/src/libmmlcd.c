#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "libmmlcd.h"

/* --- Macros ------------------------------------------ */

#define DEFAULT_MMLCD_ADDRESS 0u
#define DEFAULT_BTN_STATE_POLL_INTERVAL 50u /* ms */

/* --- Private functions declarations ------------------ */

void* btn_poll_thread(void* unused);

/* --- Global variables -------------------------------- */

static liblcd_ipc_mmlcd_addr g_mmlcd_addr = DEFAULT_MMLCD_ADDRESS;

static int g_lcd_daemon_socket = -1;

static liblcd_btn_event_callback g_btn_ev_cbk = NULL;
static bool g_is_btn_poll_th_runinning = false;
static pthread_t g_btn_poll_th_id = -1;

static liblcd_ipc_btn_state g_prev_btn_state[liblcd_ipc_btn_count];

/* --- Public functions definitions -------------------- */

/* initializes objects and lcd */
int liblcd_init(void){
    g_lcd_daemon_socket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (0 > g_lcd_daemon_socket) {
        perror("socket");
		return -1;
	}

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LIBLCD_IPC_SOCKET_PATH);
    if (0 > connect(g_lcd_daemon_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect");
        return -1;
    }

    return 0;
}

void liblcd_deinit(void){
    close(g_lcd_daemon_socket);
}

void liblcd_set_addr(liblcd_ipc_mmlcd_addr addr){
    g_mmlcd_addr = addr;
}

int liblcd_display_string_pos(const char * const string, uint8_t line, uint8_t pos){
    liblcd_ipc_print_req req = {0};
    req.head.cmd = liblcd_ipc_cmd_print;
    req.head.addr = g_mmlcd_addr;


    /* copy string to the request buffer */
    strncpy(req.params.str, string, LIBLCD_IPC_PRINT_STRING_SIZE - 1u /* subtract 1 to assure proper null termination */);
    req.params.line = line;
    req.params.pos = pos;

    int res = write(g_lcd_daemon_socket, &req, sizeof(req));
    if(0 > res){
        perror("write");
        return -1;
    }

    return 0;
}

/* clear lcd and set to home */
int liblcd_clear() {
    liblcd_ipc_req_head head = {0};
    head.cmd = liblcd_ipc_cmd_clear;
    head.addr = g_mmlcd_addr;

    if(0 > write(g_lcd_daemon_socket, &head, sizeof(head))){ 
        perror("write");
        return -1;
    }
    
	return 0;
}

/* backlight on/off */
int liblcd_backlight(bool enable){
    liblcd_ipc_backlight_req req = {
        .head = {
            .addr = g_mmlcd_addr,
            .cmd = liblcd_ipc_cmd_backlight,
        },
        .params = {.enable = (uint8_t) enable}
    };

    if(0 > write(g_lcd_daemon_socket, &req, sizeof(req))){ 
        perror("write");
        return -1;
    }

    return 0;
}

int liblcd_get_btn_state(liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state){
    if(!(state)){
        return -1;
    }

    liblcd_ipc_get_btn_state_req req = {
        .head = {
            .addr = g_mmlcd_addr,
            .cmd = liblcd_ipc_cmd_get_btn_state,
        },
        .params = {.idx = idx}
    };

    if(0 > write(g_lcd_daemon_socket, &req, sizeof(req))){ 
        perror("write");
        return -1;
    }

    if(0 > read(g_lcd_daemon_socket, state, sizeof(state))){
        perror("read");
        return -1;
    }

    return 0;
}

int liblcd_register_btn_event_callback(liblcd_btn_event_callback cbk){
    liblcd_btn_event_callback prev_cbk = g_btn_ev_cbk;
    g_btn_ev_cbk = cbk;

    if(prev_cbk){
        if(!(cbk)){
            /* Stop the polling thread */
            g_is_btn_poll_th_runinning = false;
            pthread_join(g_btn_poll_th_id, NULL);
        }
    }
    else
    {
        if(cbk){
            /* Start the polling thread */
            g_is_btn_poll_th_runinning = true;
            if(pthread_create(&g_btn_poll_th_id, NULL, btn_poll_thread, NULL)){
                return -1;
            }
        }
    }

    return 0;
}

/* --- Private functions definitions -------------------- */

void* btn_poll_thread(void* unused){
    (void)unused;

    while(g_is_btn_poll_th_runinning){
        for(liblcd_ipc_btn_idx btn_idx = liblcd_ipc_btn_first; btn_idx < liblcd_ipc_btn_count; ++btn_idx){
            liblcd_ipc_btn_state btn_state = liblcd_ipc_btn_state_invalid;
            if(!(liblcd_get_btn_state(btn_idx, &btn_state)) && liblcd_ipc_btn_state_invalid > btn_state){
                if(g_prev_btn_state[btn_idx] != btn_state){
                    g_prev_btn_state[btn_idx] = btn_state;
                    if(g_btn_ev_cbk){
                        liblcd_ipc_btn_event ev = {.state = btn_state, .idx = btn_idx};
                        g_btn_ev_cbk(ev);
                    }
                }
            }
        }
        usleep(1000 * DEFAULT_BTN_STATE_POLL_INTERVAL);
    }

    return NULL;
}
