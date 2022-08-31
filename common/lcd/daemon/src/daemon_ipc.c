#include "daemon_ipc.h"
#include "lcd_cmds.h"

#include <syslog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <libmmlcd_ipc.h>

#define EPOLL_MAX_EVENTS 10
#define EPOLL_WAIT_TIMEOUT 500 /* ms */


static int listen_socket_fd = -1;
static int epoll_fd = -1;
static struct epoll_event events[EPOLL_MAX_EVENTS] = {0};
static bool is_running = false;

/* private functions declarations */
void daemon_ipc_con_add(int fd);

void daemon_ipc_con_close(int fd);

void daemon_ipc_con_in(int fd);

/* public functions definitions */

int daemon_ipc_setup(void){
    /* create socket for listening */
    listen_socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (0 > listen_socket_fd) {
		syslog(LOG_ERR, "Failed to create socket");
		return listen_socket_fd;
	}

    int flags = fcntl(listen_socket_fd, F_GETFL, 0);
    fcntl (listen_socket_fd, F_SETFL, flags | O_NONBLOCK);

    /* allow addr reuse (needed by pigpiod) */
    int opt = 1;
    setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* bind socket to the socket file*/
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LIBLCD_IPC_SOCKET_PATH);
    unlink(LIBLCD_IPC_SOCKET_PATH);
    int ret = bind(listen_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (0 > ret) {
        syslog(LOG_ERR, "Failed to bind socket");
        return ret;
    }

    /* set socket file permissions */
    ret =  chmod(LIBLCD_IPC_SOCKET_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    if(0 > ret){
        syslog(LOG_ERR, "Failed to set socket file permissions");
        return ret;
    }

    return 0;
}

int daemon_ipc_run(void){
    int ret = listen(listen_socket_fd, 5);
   
    if(0 > ret){
        perror("listen");
        syslog(LOG_ERR, "Failed to listen on socket");
        return ret;
    }

    /* setup epoll */
    epoll_fd = epoll_create1(0);
    if (0 > epoll_fd) {
        syslog(LOG_ERR, "Failed to create epoll");
        return epoll_fd;
    }

    /* add listen socket to epoll */
    struct epoll_event epoll_event = {0};
    epoll_event.events = EPOLLIN;
    epoll_event.data.fd = listen_socket_fd;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket_fd, &epoll_event);
    if (0 > ret) {
        syslog(LOG_ERR, "Failed to add listen socket to epoll");
        return ret;
    }

    /* run epoll */
    is_running = true;
    while(is_running){
        int nfds = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, EPOLL_WAIT_TIMEOUT);
        if (0 > nfds) {
            syslog(LOG_WARNING, "Failed to read epoll events");
            /* lets try again */
            continue;
        }

        for (uint32_t event_idx = 0; event_idx < (uint32_t)nfds; ++event_idx) {
            if (events[event_idx].data.fd == listen_socket_fd) {
                daemon_ipc_con_add(listen_socket_fd);
            }
            else {
                if(events[event_idx].events & EPOLLIN){
                    daemon_ipc_con_in(events[event_idx].data.fd);
                }
                if(events[event_idx].events & (EPOLLRDHUP | EPOLLHUP)){
                    daemon_ipc_con_close(events[event_idx].data.fd);
                }
            }
        }
    }

    daemon_ipc_cleanup();

    return 0;
}

void daemon_ipc_stop(void){
    syslog(LOG_INFO, "Stopping epoll");
    is_running = false;
}

void daemon_ipc_cleanup(void){
    close(epoll_fd);
    close(listen_socket_fd);
    unlink(LIBLCD_IPC_SOCKET_PATH);
}

/* private functions definitions */

void daemon_ipc_con_add(int fd){
    syslog(LOG_INFO, "Opening new connection");

    struct sockaddr_in client_address = {0};
    struct epoll_event epoll_event = {0};
    uint32_t client_address_len = sizeof(client_address);
    int event_socket = accept(fd,
                        (struct sockaddr *) &client_address, &client_address_len);
    if (0 > event_socket) {
        syslog(LOG_ERR, "Failed to accept incoming connection");
        return;
    }
    
    int flags = fcntl(event_socket, F_GETFL, 0);
    fcntl (event_socket, F_SETFL, flags | O_NONBLOCK);

    //register new socket to epoll
    epoll_event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
    epoll_event.data.fd = event_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_socket,
                &epoll_event) == -1) {
        syslog(LOG_ERR, "Failed to add client socket to epoll");
        close(event_socket);
        return;
    }

    syslog(LOG_INFO, "New connection established? - fd: %d", event_socket);
}

void daemon_ipc_con_close(int fd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close (fd);
    syslog(LOG_INFO, "Connection closed - fd: %d", fd);
}

void daemon_ipc_con_in(int fd){
    syslog(LOG_INFO, "Connection in - fd: %d", fd);
    (void)lcd_cmds_handle_request(fd);
}
