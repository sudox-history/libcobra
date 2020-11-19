#ifndef СOBRA_SERVER_H
#define COBRA_SERVER_H

#ifdef COBRA_SERVER_PRIVATE
#include <uv.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define COBRA_SOCKET_PRIVATE
#include "socket.h"

#endif

#define COBRA_SERVER_OK 0
#define COBRA_SERVER_ERR_ALREADY_LISTENING 1
#define COBRA_SERVER_ERR_RESOLVING 2
#define COBRA_SERVER_ERR_BINDING 3
#define COBRA_SERVER_ERR_LISTENING 4
#ifdef COBRA_SERVER_PRIVATE
#define COBRA_SERVER_BACKLOG 128
#endif

typedef struct cobra_server_t cobra_server_t;

typedef void (*cobra_server_connection_cb)
        (cobra_server_t *server, cobra_socket_t *socket);

typedef void (*cobra_server_close_cb)
        (cobra_server_t *server, int error);

#ifdef COBRA_SERVER_PRIVATE
struct cobra_server_t {
    /* Libuv */
    uv_loop_t loop;
    uv_tcp_t tcp_handle;

    /* Info */
    char *host;
    int port;

    /* Management */
    bool is_listening;
    bool is_closing;
    int close_error;
    int socket_write_queue_size;

    /* Callbacks */
    cobra_server_connection_cb on_connection;
    cobra_server_close_cb on_close;

    /* Other */
    void *data;
};
#endif

cobra_server_t *cobra_server_create(int socket_write_queue_size);
void cobra_server_destroy(cobra_server_t *server);

int cobra_server_listen(cobra_server_t *server, char *host, int port);
int cobra_server_close(cobra_server_t *server);

void cobra_server_set_data(cobra_server_t *server, void *data);
void *cobra_server_get_data(cobra_server_t *server);

void cobra_server_set_callbacks(cobra_server_t *server,
                                cobra_server_connection_cb on_connection,
                                cobra_server_close_cb on_close);

#endif //COBRA_SERVER_H
