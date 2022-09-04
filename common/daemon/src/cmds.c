#include "cmds.h"
#include "hal.h"
#include "ipc_helpers.h"
#include "log.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <libmmlcd_ipc.h>

/* private functions declarations */
int cmds_handle_print(int client_fd, liblcd_ipc_mmlcd_addr addr);
int cmds_handle_clear(int client_fd, liblcd_ipc_mmlcd_addr addr);
int cmds_handle_backlight(int client_fd, liblcd_ipc_mmlcd_addr addr);
int cmds_handle_get_btn_state(int client_fd, liblcd_ipc_mmlcd_addr addr);

/* public functions definitions */
int cmds_init(void){
    if(hal_init()){
        slog(LOG_ERR, "Failed to initialize mmlcd HAL");
        return -1;
    }
    return 0;
}

void cmds_deinit(void){
    hal_deinit();
}

int cmds_handle_request(int client_fd){
    int res = 0;

    liblcd_ipc_req_head head = {.addr = 0, .cmd = liblcd_ipc_cmd_invalid};
    uint32_t read_size = ipc_socket_timeout_read(client_fd, &head, sizeof(head));
    if(0u == read_size){
        /* Connection closed by the client - do nothing */
        return 0;
    }
    else if(sizeof(head) != read_size){
        slog(LOG_ERR, "Failed to read client request command");
        return -1;
    }

    slog(LOG_DEBUG, "Client request received for command: %d", head.cmd);

    switch(head.cmd){
        case liblcd_ipc_cmd_print:{
            res = cmds_handle_print(client_fd, head.addr);
            break;
        }
        case liblcd_ipc_cmd_clear:{
            res = cmds_handle_clear(client_fd, head.addr);
            break;
        }
        case liblcd_ipc_cmd_backlight:{
            res = cmds_handle_backlight(client_fd, head.addr);
            break;
        }
        case liblcd_ipc_cmd_get_btn_state:{
            res = cmds_handle_get_btn_state(client_fd, head.addr);
            break;
        }
        default:{
            slog(LOG_ERR, "Invalid command received");
            res = -1;
        }
    }

    return res;
}

/* private functions definittions */

int cmds_handle_print(int client_fd, liblcd_ipc_mmlcd_addr addr){
    liblcd_ipc_print_params req = {0};
    
    if(ipc_read_req_params(client_fd, &req, sizeof(req))){
        slog(LOG_ERR, "Failed to read client print request parameters");
        return -1;
    }
    
    return hal_lcd_display_string_pos(addr, req.str, req.line, req.pos);
}
int cmds_handle_clear(int client_fd, liblcd_ipc_mmlcd_addr addr){
    (void)client_fd;
    return hal_lcd_clear(addr);
}
int cmds_handle_backlight(int client_fd, liblcd_ipc_mmlcd_addr addr){
    liblcd_ipc_backlight_params req = {0};

    if(ipc_read_req_params(client_fd, &req, sizeof(req))){
        slog(LOG_ERR, "Failed to read client backlight request parameters");
        return -1;
    }

    return hal_lcd_backlight(addr, req.enable);
}

int cmds_handle_get_btn_state(int client_fd, liblcd_ipc_mmlcd_addr addr){
    liblcd_ipc_get_btn_state_params req = {0};
    liblcd_ipc_btn_state state = liblcd_ipc_btn_state_invalid;
    int res = 0;


    if(ipc_read_req_params(client_fd, &req, sizeof(req))){
        slog(LOG_ERR, "Failed to read get button state request parameters");
        return -1;
    }

    if((res = hal_btn_get_state(addr, req.idx, &state))){
        slog(LOG_ERR, "Failed to read button state");
        return res;
    }

    slog(LOG_DEBUG, "Read button state: %d", state);

    res = write(client_fd, &state, sizeof(state));
    if(0 > res){
        slog(LOG_ERR, "Failed to write button state to the client");
        return res;
    }

    return 0;
}