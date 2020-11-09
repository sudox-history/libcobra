#ifndef COBRA_TCP_CONNECTION_H
#define COBRA_TCP_CONNECTION_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <uv.h>

#include "buffer.h"

#define COBRA_TCP_CONNECTION_OK 0
#define COBRA_TCP_CONNECTION_ERR_RESOLUTION 1
#define COBRA_TCP_CONNECTION_ERR_CONNECTING 2
#define COBRA_TCP_CONNECTION_ERR_ALREADY_CONNECTED 3
#define COBRA_TCP_CONNECTION_ERR_NOT_CONNECTED 4
#define COBRA_TCP_CONNECTION_ERR_WRITING 5
#ifdef COBRA_TCP_CONNECTION_PRIVATE
#define COBRA_TCP_CONNECTION_MAX_PACKET_SIZE 65535
#endif

typedef struct cobra_tcp_connection_t cobra_tcp_connection_t;

typedef void (*connection_connect_cb)(cobra_tcp_connection_t *connection);
typedef void (*connection_close_cb)(cobra_tcp_connection_t *connection, int status);
typedef void (*connection_data_cb)(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len);
typedef void (*connection_activity_cb)(cobra_tcp_connection_t *connection);

typedef uv_buf_t uv_buf;
#ifdef COBRA_TCP_CONNECTION_PRIVATE
void on_read(uv_stream_t *stream, ssize_t read_len, const uv_buf_t *_);
void on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *buf);
// TODO: Rewrite usage of requests
struct cobra_tcp_connection_t {
    /* Main loop */
    uv_loop_t loop;

    /* Handlers */
    uv_tcp_t tcp_handle;

    /* Requests */
    uv_getaddrinfo_t getaddrinfo_req;
    uv_connect_t connect_req;
    uv_write_t write_req;

    /* Connection management */
    bool connected;
    cobra_buffer_t *buffer;
    uint16_t packet_len;

    /* Callbacks */
    connection_connect_cb on_connect;
    connection_close_cb on_close;
    connection_data_cb on_data;
    connection_activity_cb on_activity;
    void *data;
};
#endif

cobra_tcp_connection_t *cobra_tcp_connection_create();
void cobra_tcp_connection_destroy(cobra_tcp_connection_t *connection);

void cobra_tcp_connection_set_callbacks(cobra_tcp_connection_t *connection,
                                        connection_connect_cb on_connect,
                                        connection_close_cb on_close,
                                        connection_data_cb on_data,
                                        connection_activity_cb on_activity);

int cobra_tcp_connection_connect(cobra_tcp_connection_t *connection, char *host, char *port);
int cobra_tcp_connection_close(cobra_tcp_connection_t *connection);

void cobra_tcp_connection_set_data(cobra_tcp_connection_t *connection, void *data);
void *cobra_tcp_connection_get_data(cobra_tcp_connection_t *connection);

int cobra_tcp_connection_send(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len);

#endif //COBRA_TCP_CONNECTION_H
