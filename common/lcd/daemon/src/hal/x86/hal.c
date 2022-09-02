#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "libmmlcd_ipc.h"
#include "log.h"

#define MMLCD_DEV_PATH "/dev/mmlcd0"
#define MMLCD_PROC_PATH_BTN1 "/proc/mmlcd0_btn1_state"
#define MMLCD_PROC_PATH_BTN2 "/proc/mmlcd0_btn2_state"
#define MMLCD_PROC_PATH_BTN3 "/proc/mmlcd0_btn3_state"


static const char *btn_proc_paths[liblcd_ipc_btn_count] = { 
	MMLCD_PROC_PATH_BTN1,
	MMLCD_PROC_PATH_BTN2,
	MMLCD_PROC_PATH_BTN3
};

static int open_mmlcd_file(const char * const path){
	int mmlcd_dev_fd = open(path, O_RDWR);
	if (0 > mmlcd_dev_fd) {
		perror("open");
		slog(LOG_ERR, "Failed to open mmlcd device file '%s', code: %d", path, mmlcd_dev_fd);
		return -1;
	}

	return mmlcd_dev_fd;
}

int hal_lcd_init(void){
	/* Nothing to be done */
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

	int mmlcd_dev_fd = open_mmlcd_file(MMLCD_DEV_PATH);
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
		slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
		close(mmlcd_dev_fd);
		return -1;
	}

	close(mmlcd_dev_fd);

	return 0;
}

int hal_lcd_clear(int unused){
	(void) unused;

	int mmlcd_dev_fd = open_mmlcd_file(MMLCD_DEV_PATH);
	if(0 > mmlcd_dev_fd){
		return -1;
	}
	
	liblcd_ipc_cmd cmd = liblcd_ipc_cmd_clear;

	int res = -1;
	if(0 > (res = write(mmlcd_dev_fd, &cmd, sizeof(cmd)))){ 
		perror("write");
		slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
		close(mmlcd_dev_fd);
		return -1;
	}

	close(mmlcd_dev_fd);
	
	return 0;
}

int hal_lcd_backlight(int unused, bool state){
	(void) unused;

	int mmlcd_dev_fd = open_mmlcd_file(MMLCD_DEV_PATH);
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
		slog(LOG_ERR, "Failed to write mmlcd device file, code: %d", res);
		close(mmlcd_dev_fd);
		return -1;
	}

	close(mmlcd_dev_fd);

	return 0;
}

int hal_btn_init(void){
	/* Nothing to be done */
	return 0;
}

void hal_btn_deinit(void){
	/* Nothing to be done */
}

int hal_btn_get_state(liblcd_ipc_btn_idx idx, liblcd_ipc_btn_state *state){
	if(!(state) || liblcd_ipc_btn_idx_invlid <= idx){
		return -1;
	}

	FILE *proc_fp = fopen(btn_proc_paths[idx], "r");
	if(!(proc_fp)){
		perror("fopen");
		slog(LOG_ERR, "Failed to open mmlcd proc file '%s'", btn_proc_paths[idx]);
		return -1;
	}

	if(0 >= fscanf(proc_fp, "%u\n", state)){
		perror("fscanf");
		slog(LOG_ERR, "Failed to read mmlcd proc file '%s'", btn_proc_paths[idx]);
		fclose(proc_fp);
		return -1;	
	}

	fclose(proc_fp);

	return 0;
}