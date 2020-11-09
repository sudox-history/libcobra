#ifndef COBRA_TCP_SERVER_H
#define COBRA_TCP_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>

#define COBRA_TCP_CONNECTION_PRIVATE
#include "tcp_connection.h"

#ifdef COBRA_TCP_SERVER_PRIVATE
#define COBRA_TCP_SERVER_OK 0
#define COBRA_TCP_SERVER_ERR_ALREADY_LISTENING 1
#define COBRA_TCP_SERVER_ERR_NOT_LISTENING 1
#define COBRA_TCP_SERVER_ERR_BINDING 3
#define COBRA_TCP_SERVER_ERR_LISTENING 4
#endif

typedef struct cobra_tcp_server_t cobra_tcp_server_t;

typedef void (*server_connection_cb)(cobra_tcp_server_t *server, cobra_tcp_connection_t *connection);
typedef void (*server_close_cb)(cobra_tcp_server_t *server, int status);

#ifdef COBRA_TCP_SERVER_PRIVATE
// TODO: Rewrite usage of requests
struct cobra_tcp_server_t {
    /* Main loop */
    uv_loop_t loop;

    /* Handles */
    uv_tcp_t tcp_handle;

    /* Management */
    bool listening;

    /* Callbacks */
    server_connection_cb on_connection;
    server_close_cb on_close;

    void *data;
};
#endif

cobra_tcp_server_t *cobra_server_create();
void cobra_tcp_server_destroy(cobra_tcp_server_t *server);

void cobra_tcp_server_set_callbacks(cobra_tcp_server_t *server,
                                    server_connection_cb on_connection,
                                    server_close_cb on_close);

int cobra_tcp_server_listen(cobra_tcp_server_t *server, char *host, int port);
int cobra_tcp_server_close(cobra_tcp_server_t *server);

void cobra_tcp_server_set_data(cobra_tcp_server_t *server, void *data);
void* cobra_tcp_server_get_data(cobra_tcp_server_t *server);

#endif //COBRA_TCP_SERVER_H