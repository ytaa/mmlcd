#include <stdio.h>
#include <libmmlcd.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static const char **g_progs = NULL;
static pid_t g_child_pid = 0;

static void btn_event_callback(liblcd_ipc_btn_event ev);

int main(int argc, const char **argv){
    if(0 > liblcd_init()){
        fprintf(stderr, "Failed to initialize libmmlcd\n");
        return 1;
    }

    if(4 != argc){
        printf("Usage: %s prog1 prog2 prog3\n", argv[0]);
        return -1;
    }

    g_progs = argv+1;

    liblcd_register_btn_event_callback(btn_event_callback);

    while(1){usleep(100000);}

    return 0;
}

static void btn_event_callback(liblcd_ipc_btn_event ev){
    if(liblcd_ipc_btn_released == ev.state){
        if(0 < g_child_pid){
            kill(g_child_pid, SIGTERM);
        }

        pid_t pid = fork();

        if(0 == pid){
            /* child */
            execl(g_progs[ev.idx], g_progs[ev.idx], NULL);
            exit(0);
        }
            
        g_child_pid = pid;
    }
}