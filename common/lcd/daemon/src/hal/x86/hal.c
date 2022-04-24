#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#include "libmmlcd_ipc.h"

#define MMLCD_DEV_PATH "/dev/mmlcd0"

static int open_mmlcd_dev(const char * const path){
    int mmlcd_dev_fd = open(path, O_RDWR);
    syslog(LOG_INFO, "hi11");
    if (0 > mmlcd_dev_fd) {
        syslog(LOG_INFO, "hi12 %d", mmlcd_dev_fd);
        perror("open");
        syslog(LOG_ERR, "Failed to open mmlcd device file '%s', code: %d", path, mmlcd_dev_fd);
		return -1;
	}
    syslog(LOG_INFO, "hi14 %d", mmlcd_dev_fd);

    return mmlcd_dev_fd;
}

int hal_lcd_init(void){
    /* For now we are supporting only mmlcd0 */
    return 0;
}

void hal_lcd_deinit(int mmlcd_dev_fd){
    close(mmlcd_dev_fd);
}

int hal_lcd_setup(int mmlcd_dev_fd){
    (void)mmlcd_dev_fd;
    return 0;
}

int hal_lcd_display_string_pos(int unused, const char * const string, uint8_t line, uint8_t pos){
    (void) unused;

    int mmlcd_dev_fd = open_mmlcd_dev(MMLCD_DEV_PATH);
    if(0 > mmlcd_dev_fd){
        return -1;
    }

    liblcd_ipc_print_req req = {0};
    req.cmd = liblcd_ipc_cmd_print;

    /* copy string to the request buffer */
    strncpy(req.params.str, string, LIBLCD_IPC_PRINT_STRING_SIZE - 1u /* subtract 1 to assure proper null termination */);
    req.params.line = line;
    req.params.pos = pos;
    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &req, sizeof(req)))){ 
        perror("write");
        syslog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);

    return 0;
}

int hal_lcd_clear(int unused){
    (void) unused;

    int mmlcd_dev_fd = open_mmlcd_dev(MMLCD_DEV_PATH);
    if(0 > mmlcd_dev_fd){
        return -1;
    }
    
    liblcd_ipc_cmd cmd = liblcd_ipc_cmd_clear;

    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &cmd, sizeof(cmd)))){ 
        perror("write");
        syslog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);
    
	return 0;
}

int hal_lcd_backlight(int unused, bool state){
    (void) unused;

    int mmlcd_dev_fd = open_mmlcd_dev(MMLCD_DEV_PATH);
    if(0 > mmlcd_dev_fd){
        return -1;
    }

    liblcd_ipc_backlight_req req = {
        .cmd = liblcd_ipc_cmd_backlight,
        .params = {.enable = (uint8_t) state}
    };

    int res = -1;
    if(0 > (res = write(mmlcd_dev_fd, &req, sizeof(req)))){ 
        perror("write");
        syslog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
        close(mmlcd_dev_fd);
        return -1;
    }

    close(mmlcd_dev_fd);

    return 0;
}