#ifndef DAEMON_IPC_H
#define DAEMON_IPC_H

int daemon_ipc_setup(void);

int daemon_ipc_run(void);

void daemon_ipc_stop(void);

void daemon_ipc_cleanup(void);

#endif /* DAEMON_IPC_H */