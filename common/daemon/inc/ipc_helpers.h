#ifndef IPC_HELPERS
#define IPC_HELPERS

#include <stdint.h>
#include <stdlib.h>

/* --- Macros and constants ---------------------------- */

/* --- Type definitions -------------------------------- */

/* --- Extern variables -------------------------------- */

/* --- Public functions declarations ------------------- */

int ipc_read_req_params(int client_fd, void *params, const unsigned int size);

uint32_t ipc_socket_timeout_read(int fd, void * receive_buffer, uint32_t expected_size);

#endif