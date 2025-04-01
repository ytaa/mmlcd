#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <systemd/sd-daemon.h>
#include <syslog.h>
#include <libmmlcd.h>
#include <libmmlcd_ipc.h>
#include <sys/socket.h>

/* kernel headers */
#include <tools/le_byteshift.h>
#include <functionfs.h>

/* --- Macros ------------------------------------------ */

#define EP_CTRL_BUF_SIZE (4 * sizeof(struct usb_functionfs_event))
#define EP_CMD_BUF_SIZE (8 * 1024)

/* --- Type definitions -------------------------------- */

enum ep_addr{
    EP_ADDR_CTRL = 0,
    EP_ADDR_BTN,
    EP_ADDR_CMD,
    EP_ADDR_NUM,
};

enum read_socket_thread_id {
    RD_SOC_TH_ID_CONTROL_EP = 0,
    RD_SOC_TH_ID_CMD_EP,
    RD_SOC_TH_ID_NUM
};

struct read_socket_thread {
    const char *name;
    int fd;
    pthread_t id;
    ssize_t (*callback)(struct read_socket_thread *, size_t);
    void *buffer;
    size_t buffer_size;
};

/* --- Private functions declarations ------------------ */

static void *read_socket_thread(void* arg);
static ssize_t handle_control_ep(struct read_socket_thread *t, size_t size);
static ssize_t handle_cmd_ep(struct read_socket_thread *t, size_t size);
static void handle_setup_request(const struct usb_ctrlrequest *setup);
static void btn_event_callback(liblcd_ipc_btn_event ev);

/* --- Global variables -------------------------------- */

static char ctrl_ep_buf[EP_CTRL_BUF_SIZE];
static char cmd_ep_buf[EP_CMD_BUF_SIZE];

static int g_mmlcd_fd = -1;

struct read_socket_thread read_socket_threads[RD_SOC_TH_ID_NUM] = {
    /* ep0 - control endpoint */
    {
        .name = "control endpoint",
        .fd = SD_LISTEN_FDS_START + EP_ADDR_CTRL,
        .id = -1,
        .callback = handle_control_ep,
        .buffer = ctrl_ep_buf,
        .buffer_size = sizeof(ctrl_ep_buf)
    },
    /* ep1 - bulk out endpoint - incoming commands */
    {
        .name = "command endpoint",
        .fd = SD_LISTEN_FDS_START + EP_ADDR_CMD,
        .id = -1,
        .callback = handle_cmd_ep,
        .buffer = cmd_ep_buf,
        .buffer_size = sizeof(cmd_ep_buf)
    }
};

/* --- Public functions definitions ------------------- */

int main(int argc, char **argv){
    if(argc <= 0){
        return -1;
    }

    printf("Starting %s...\n", argv[0]);
    syslog(LOG_INFO, "Starting %s...", argv[0]);
    openlog(argv[0], LOG_PID, LOG_DAEMON);

    /* Validate socket descriptors num inherited from systemd */
    if(EP_ADDR_NUM != sd_listen_fds(0))
    {
        syslog(LOG_ERR, "Failed to inherit descriptors from systemd, sd_listen_fds(0) = %d", sd_listen_fds(0));
    }

    /* initialize libmmcd */
    if(liblcd_init()){
        syslog(LOG_ERR, "Failed to initialize libmmlcd");
        return -1;
    }

    /* Create a socket for communication with mmlcdd */
    g_mmlcd_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (0 > g_mmlcd_fd) {
        syslog(LOG_ERR, "Failed to create socket for mmlcdd");
        return -1;
    }

    /* Connect the socket to mmlcdd UDS */
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LIBLCD_IPC_SOCKET_PATH);
    if (0 > connect(g_mmlcd_fd, (struct sockaddr *)&addr, sizeof(addr))) {
        syslog(LOG_ERR, "Failed to connect socket for mmlcdd");
        return -1;
    }

    /* register callback for handlig of button events */
    liblcd_register_btn_event_callback(btn_event_callback);

    /* Start communication threads */
    int ret = pthread_create(&read_socket_threads[RD_SOC_TH_ID_CMD_EP].id, NULL, read_socket_thread, &read_socket_threads[RD_SOC_TH_ID_CMD_EP]);
    if(0 > ret){
        syslog(LOG_ERR, "Failed to create thread for command endpoint");
        return -1;
    }

    /* Use main thread for handling of control ep */
    read_socket_thread(&read_socket_threads[RD_SOC_TH_ID_CONTROL_EP]);

    /* Wait until cmd ep communication thread is finished */
    pthread_join(read_socket_threads[RD_SOC_TH_ID_CMD_EP].id, NULL);

    closelog();

    return 0;
}

/* --- Private functions definitions ------------------- */

static void *read_socket_thread(void* arg){
    struct read_socket_thread *t = (struct read_socket_thread *)arg;

    int ret = -1;

    while (1) {
        pthread_testcancel();

        ret = read(t->fd, t->buffer, t->buffer_size);
        if (ret > 0) {
            ret = t->callback(t, ret);
            if (ret < 0) {
                syslog(LOG_ERR, "[th: '%s'] Read callback failed: %d", t->name, ret);
            }
        } 
        else if (errno == EINTR || errno == EAGAIN) {
            perror("read");
            syslog(LOG_ERR, "[th: '%s'] Failed to read fd with code: %d. Retrying...", t->name, ret);
        }
        else {
            perror("read");
            syslog(LOG_ERR, "[th: '%s'] Failed to read fd with code: %d. Aborting.", t->name, ret);
            break;
        }
    }

    return NULL;
}

static ssize_t handle_control_ep(struct read_socket_thread *t, size_t size){
    static const char *const names[] = {
        [FUNCTIONFS_BIND] = "BIND",
        [FUNCTIONFS_UNBIND] = "UNBIND",
        [FUNCTIONFS_ENABLE] = "ENABLE",
        [FUNCTIONFS_DISABLE] = "DISABLE",
        [FUNCTIONFS_SETUP] = "SETUP",
        [FUNCTIONFS_SUSPEND] = "SUSPEND",
        [FUNCTIONFS_RESUME] = "RESUME",
    };

    const struct usb_functionfs_event *event = t->buffer;
    size_t n;

    for (n = size / sizeof(*event); n; --n, ++event)
        switch (event->type) {
            case FUNCTIONFS_BIND:
            case FUNCTIONFS_UNBIND:
            case FUNCTIONFS_ENABLE:
            case FUNCTIONFS_DISABLE:
            case FUNCTIONFS_SETUP:
            case FUNCTIONFS_SUSPEND:
            case FUNCTIONFS_RESUME:{
                syslog(LOG_INFO, "USB Event %s\n", names[event->type]);
                if (event->type == FUNCTIONFS_SETUP){
                    handle_setup_request(&event->u.setup);
                }
                break;
            }

            default:
                printf("Event %03u (unknown)\n", event->type);
        }

    return size;
}

static ssize_t handle_cmd_ep(struct read_socket_thread *t, size_t size){
    return write(g_mmlcd_fd, t->buffer, size);
}

static void handle_setup_request(const struct usb_ctrlrequest *setup) {
    syslog(LOG_INFO, "Received USB setup request:");
    syslog(LOG_INFO, "bRequestType = %d", setup->bRequestType);
    syslog(LOG_INFO, "bRequest     = %d", setup->bRequest);
    syslog(LOG_INFO, "wValue       = %d", setup->wValue);
    syslog(LOG_INFO, "wIndex       = %d", setup->wIndex);
    syslog(LOG_INFO, "wLength      = %d", setup->wLength);
}

static void btn_event_callback(liblcd_ipc_btn_event ev){
    syslog(LOG_INFO, "Received button event: {idx: %d, state: %d}", ev.idx, ev.state);

    int int_in_fd = SD_LISTEN_FDS_START + EP_ADDR_BTN;
    int ret = write(int_in_fd, &ev, sizeof(ev));
    if(0 > ret){
        perror("write");
        syslog(LOG_ERR, "[th: 'buttons endpoint'] Failed to write fd with code: %d.", ret);
    }
    else if(sizeof(ev) != ret){
        syslog(LOG_ERR, "[th: 'buttons endpoint'] Failed to write full payload to fd - bytes writen: %d.", ret);
    }
    else{
        /* Write successfull - do nothing */
    }
}