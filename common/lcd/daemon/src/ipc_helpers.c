#include <time.h>
#include <unistd.h>

#include "ipc_helpers.h"
#include "log.h"

/* --- Macros ------------------------------------------ */

#define MAX_READ_TIMEOUT_MS 1000.f

/* --- Private functions declarations ------------------ */

/* --- Global variables -------------------------------- */

/* --- Public functions definitions -------------------- */

int ipc_read_req_params(int client_fd, void *params, const unsigned int size){
    uint32_t read_size = ipc_socket_timeout_read(client_fd, params, size);
    if(0u == read_size){
        /* Connection closed by the client - do nothing */
        return 0;
    }
    else if(size != read_size){
        slog(LOG_ERR, "Missmatch in read size - expected %u, acutal %u", size, read_size);
        return -1;
    }

    return 0;
}

uint32_t ipc_socket_timeout_read(int fd, void * receive_buffer, uint32_t expected_size){
    clock_t start_read_time = clock();
    clock_t current_read_time;

    uint32_t received_data_size = 0;
    int64_t currentReadDataSize = 0;

    while(received_data_size < expected_size){
        currentReadDataSize = read(fd, receive_buffer + received_data_size, expected_size-received_data_size);
        if(currentReadDataSize > 0){
            //last_read_time = clock();
            received_data_size += currentReadDataSize;
            current_read_time = clock();
            double deltaTime = (double)(current_read_time - start_read_time)/CLOCKS_PER_SEC*1000.f;
            if(deltaTime > MAX_READ_TIMEOUT_MS){
                slog(LOG_ERR, "Socket read timeout");
                return received_data_size;
            }
        }
        else{
            //slog(LOG_ERR, "Reading aborted before reaching expected size (code %lld)", currentReadDataSize);
            return received_data_size;
        }
    }

    return received_data_size;
}