#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sched.h>
#include <stdbool.h>

#include "daemon_ipc.h"
#include "lcd_cmds.h"

/*
 * Close our association with syslog before exiting
 */
static void cleanup()
{

    daemon_ipc_cleanup();
	lcd_cmds_deinit();
	closelog();
}

/*
 * On SIGINT, exit the main loop
 */
static void sig_handler(int signo)
{
    switch(signo){
        case SIGINT:
        case SIGTERM:{
            daemon_ipc_stop();
            break;
        }
        default:{
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
	(void)argv;
	//daemonize(argv[0]);

	openlog("lcdd", LOG_NOWAIT|LOG_PID, LOG_USER);
	atexit(&cleanup);
	syslog(LOG_NOTICE, "Starting daemon...\n");

	/* set up the signal handler */
	if (signal(SIGINT, &sig_handler) == SIG_ERR) {
		syslog(LOG_ERR, "Failed to register SIGINT handler, quitting...");
		exit(EXIT_FAILURE);
	}
    if (signal(SIGTERM, &sig_handler) == SIG_ERR) {
		syslog(LOG_ERR, "Failed to register SIGTERM handler, quitting...");
		exit(EXIT_FAILURE);
	}

	if(0 > lcd_cmds_init()){
		syslog(LOG_ERR, "Failed to initialize LCD command handler, quitting...");
		exit(EXIT_FAILURE);
	}

    if( 0 > daemon_ipc_setup()){
		syslog(LOG_ERR, "Failed to initialize daemon ipc, quitting...");
		exit(EXIT_FAILURE);
	}

	/* start the loop */
	syslog(LOG_NOTICE, "Entering main loop");

    if( 0 > daemon_ipc_run()){
		syslog(LOG_ERR, "Daemon ipc exited with unexpected value");
	}

	syslog(LOG_NOTICE, "Exiting...");
	cleanup();
	return 0;
}