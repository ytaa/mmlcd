#ifndef CMDS_H
#define CMDS_H

int cmds_init(void);

void cmds_deinit(void);

int cmds_handle_request(int client_fd);

#endif /* CMDS_H */