#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <stdint.h>
#include <unistd.h>

#include "liblcd.h"
#include "liblcd_ipc.h"

#define LISTEN_SOCKET_PATH "/tmp/lcd_daemon.sock"

static int lcd_daemon_socket = -1;

/* initializes objects and lcd */
int liblcd_init(void){
    lcd_daemon_socket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (0 > lcd_daemon_socket) {
        perror("socket");
		return -1;
	}

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LISTEN_SOCKET_PATH);
    if (0 > connect(lcd_daemon_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect");
        return -1;
    }

    return 0;
}

void liblcd_deinit(void){
    close(lcd_daemon_socket);
}

int liblcd_display_string_pos(const char * const string, uint8_t line, uint8_t pos){
    liblcd_ipc_print_req req = {0};
    req.cmd = liblcd_ipc_cmd_print;

    /* copy string to the request buffer */
    strncpy(req.params.str, string, LIBLCD_IPC_PRINT_STRING_SIZE - 1u /* subtract 1 to assure proper null termination */);
    req.params.line = line;
    req.params.pos = pos;

    if(0 > write(lcd_daemon_socket, &req, sizeof(req))){ 
        perror("write");
        return -1;
    }

    return 0;
}

/* clear lcd and set to home */
int liblcd_clear() {
    liblcd_ipc_cmd cmd = liblcd_ipc_cmd_clear;

    if(0 > write(lcd_daemon_socket, &cmd, sizeof(cmd))){ 
        perror("write");
        return -1;
    }
    
	return 0;
}

/* backlight on/off */
int liblcd_backlight(bool enable){
    liblcd_ipc_backlight_req req = {
        .cmd = liblcd_ipc_cmd_backlight,
        .params = {.enable = (uint8_t) enable}
    };

    if(0 > write(lcd_daemon_socket, &req, sizeof(req))){ 
        perror("write");
        return -1;
    }

    return 0;
}

/* add custom characters (0 - 7) liblcd_ipc_cmd_backlight
def lcd_load_custom_chars(self, fontdata):
    self.lcd_write(0x40);
    for char in fontdata:
        for line in char:
        self.lcd_write_char(line) 
*/