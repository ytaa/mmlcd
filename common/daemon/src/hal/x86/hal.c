#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "libmmlcd_ipc.h"
#include "log.h"

/* --- Macros ------------------------------------------ */

#define MMLCD_DEV_FMT "/dev/mmlcd%u"
#define MMLCD_DEV_MAX_PATH_SIZE 20u
#define MMLCD_PROC_BTN_FMT "/proc/mmlcd%u_btn%u_state"
#define MMLCD_PROC_BTN_MAX_PATH_SIZE 40u

/* --- Private functions declarations ------------------ */

static int open_mmlcd_dev(liblcd_ipc_mmlcd_addr addr);
static FILE* open_mmlcd_proc_btn(liblcd_ipc_mmlcd_addr addr, liblcd_ipc_btn_idx idx);

/* --- Global variables -------------------------------- */

/* --- Public functions definitions -------------------- */

int hal_init(void){
    /* Nothing to be done */
    return 0;
}

void hal_deinit(void){
    /* Nothing to be done */
}

int hal_lcd_display_string_pos(liblcd_ipc_mmlcd_addr addr, const char * const string, uint8_t line, uint8_t pos){

    int mmlcd_dev_fd = open_mmlcd_dev(addr);
    if(0 > mmlcd_dev_fd){
        return -1;
    }

    liblcd_ipc_print_req req = {0};
    req.head.cmd = liblcd_ipc_cmd_print;
    req.head.addr = addr;

    /* copy string to the request buffer */
    strncpy(req.params.str, string, LIBLCD_IPC_PRINT_STRING_SIZE - 1u /* subtract 1 to assure proper null termination */);
    req.params.line = line;
    req.params.pos = pos;
    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &req, sizeof(req)))){ 
        perror("write");
        slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);

    return 0;
}

int hal_lcd_clear(liblcd_ipc_mmlcd_addr addr){
    int mmlcd_dev_fd = open_mmlcd_dev(addr);
    if(0 > mmlcd_dev_fd){
        return -1;
    }
    
    liblcd_ipc_req_head head = {
        .cmd = liblcd_ipc_cmd_clear,
        .addr = addr,
    };

    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &head, sizeof(head)))){ 
        perror("write");
        slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);
    
    return 0;
}

int hal_lcd_backlight(liblcd_ipc_mmlcd_addr addr, bool state){
    int mmlcd_dev_fd = open_mmlcd_dev(addr);
    if(0 > mmlcd_dev_fd){
        return -1;
    }

    liblcd_ipc_backlight_req req = {
        .head = {
            .cmd = liblcd_ipc_cmd_backlight,
            .addr = addr,
        },
        .params = {.enable = (uint8_t) state}
    };

    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &req, sizeof(req)))){ 
        perror("write");
        slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);

    return 0;
}

int hal_btn_get_state(liblcd_ipc_mmlcd_addr addr, liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state){
    if(!(state) || liblcd_ipc_btn_idx_invlid <= idx){
        return -1;
    }

    FILE *proc_fp = open_mmlcd_proc_btn(addr, idx);
    if(!(proc_fp)){
        perror("fopen");
        slog(LOG_ERR, "Failed to open mmlcd proc file (addr %u, idx %u)", addr, idx);
        return -1;
    }

    if(0 >= fscanf(proc_fp, "%u\n", state)){
        perror("fscanf");
        slog(LOG_ERR, "Failed to read mmlcd proc file (addr %u, idx %u)", addr, idx);
        fclose(proc_fp);
        return -1;	
    }

    fclose(proc_fp);

    return 0;
}

/* --- Private functions definitions ------------------- */

static int open_mmlcd_dev(liblcd_ipc_mmlcd_addr addr){
    char path[MMLCD_DEV_MAX_PATH_SIZE];
    snprintf(path, sizeof(path), MMLCD_DEV_FMT, addr);

    int mmlcd_dev_fd = open(path, O_RDWR);
    if (0 > mmlcd_dev_fd) {
        perror("open");
        slog(LOG_ERR, "Failed to open mmlcd device file '%s', code: %d", path, mmlcd_dev_fd);
        return -1;
    }

    return mmlcd_dev_fd;
}

static FILE* open_mmlcd_proc_btn(liblcd_ipc_mmlcd_addr addr, liblcd_ipc_btn_idx idx){
    char path[MMLCD_PROC_BTN_MAX_PATH_SIZE];
    snprintf(path, sizeof(path), MMLCD_PROC_BTN_FMT, addr, idx+1);

    return fopen(path, "r");
}