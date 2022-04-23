#include "lcd_cmds.h"
#include "lcd.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <syslog.h>
#include <time.h>
#include <liblcd_ipc.h>

#define MAX_READ_TIMEOUT_MS 1000.f

static int lcd_dev = -1;

/* private functions declarations */
int lcd_cmds_handle_print(int client_fd);
int lcd_cmds_handle_clear(int client_fd);
int lcd_cmds_handle_backlight(int client_fd);
uint32_t daemon_ipc_socket_timeout_read(int fd, void * receive_buffer, uint32_t expected_size);

/* public functions definitions */
int lcd_cmds_init(void){
    lcd_dev = lcd_init();

    if(0 > lcd_dev){
        syslog(LOG_ERR, "Failed to open LCD device");
        return lcd_dev;
    }

    return lcd_setup(lcd_dev);
}

void lcd_cmds_deinit(void){
    lcd_deinit(lcd_dev);
    lcd_dev = -1;
}

int lcd_cmds_handle_request(int client_fd){
    int res = 0;

    liblcd_ipc_cmd cmd = liblcd_ipc_cmd_invalid;
    if(sizeof(cmd) != daemon_ipc_socket_timeout_read(client_fd, &cmd, sizeof(cmd))){
        syslog(LOG_ERR, "Failed to read client request command");
        return -1;
    }

    syslog(LOG_INFO, "Client request received for command: %d", cmd);

    switch(cmd){
        case liblcd_ipc_cmd_print:{
            res = lcd_cmds_handle_print(client_fd);
            break;
        }
        case liblcd_ipc_cmd_clear:{
            res = lcd_cmds_handle_clear(client_fd);
            break;
        }
        case liblcd_ipc_cmd_backlight:{
            res = lcd_cmds_handle_backlight(client_fd);
            break;
        }
        default:{
            syslog(LOG_ERR, "Invalid command received");
            res = -1;
        }
    }

    return res;
}

/* private functions definittions */

int lcd_cmds_handle_print(int client_fd){
    liblcd_ipc_print_params req = {0};
    if(sizeof(liblcd_ipc_print_params) != daemon_ipc_socket_timeout_read(client_fd, &req, sizeof(liblcd_ipc_print_params))){
        syslog(LOG_ERR, "Failed to read client print request parameters");
        return -1;
    }

    return lcd_display_string_pos(lcd_dev, req.str, req.line, req.pos);
}
int lcd_cmds_handle_clear(int client_fd){
    (void)client_fd;
    return lcd_clear(lcd_dev);
}
int lcd_cmds_handle_backlight(int client_fd){
    liblcd_ipc_backlight_params req = {0};
    if(sizeof(liblcd_ipc_backlight_params) != daemon_ipc_socket_timeout_read(client_fd, &req, sizeof(liblcd_ipc_backlight_params))){
        syslog(LOG_ERR, "Failed to read client backlight request parameters");
        return -1;
    }

    return lcd_backlight(lcd_dev, req.enable);
}

uint32_t daemon_ipc_socket_timeout_read(int fd, void * receive_buffer, uint32_t expected_size){
    clock_t last_read_time = clock();
    clock_t current_read_time;

    uint32_t received_data_size = 0;
    uint32_t currentReadDataSize = 0;

    while(received_data_size < expected_size){
        currentReadDataSize = read(fd, receive_buffer + received_data_size, expected_size-received_data_size);
        if(currentReadDataSize > 0){
            last_read_time = clock();
            received_data_size += currentReadDataSize;
        }
        else{
            current_read_time = clock();
            double deltaTime = (double)(current_read_time - last_read_time)/CLOCKS_PER_SEC*1000.f;
            if(deltaTime > MAX_READ_TIMEOUT_MS){
                syslog(LOG_ERR, "Socket read timeout");
                return received_data_size;
            }
        }
    }

    return received_data_size;
}