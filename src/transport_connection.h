#ifndef COBRA_TRANSPORT_CONNECTION_H
#define COBRA_TRANSPORT_CONNECTION_H

typedef struct transport_connection_t transport_connection_t;

typedef void (*transport_client_connect_cb)(transport_connection_t *connection);

typedef void (*transport_client_close_cb)(transport_connection_t *connection);

typedef void (*transport_client_error_cb)(transport_connection_t *connection, int code);

typedef void (*transport_client_packet_cb)(transport_connection_t *connection, unsigned char *data, int len);

#ifdef COBRA_TRANSPORT_CONNECTION_PRIVATE

struct transport_connection_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;
    uv_connect_t tcp_connect_req;
    transport_client_connect_cb on_connect;
    transport_client_close_cb on_close;
    transport_client_error_cb on_error;
    transport_client_packet_cb on_packet;
    transport_buffer_t *buffer;
    int current_packet_len;
};

#endif

transport_connection_t *transport_connection_create(
        transport_client_connect_cb on_connect,
        transport_client_close_cb on_close,
        transport_client_error_cb on_error,
        transport_client_packet_cb on_packet
);

void transport_connection_destroy(transport_connection_t *connection);

void transport_connection_connect(transport_connection_t *connection, char *host, char *port);

int transport_connection_send(transport_connection_t *connection, unsigned char *data, int len);

int transport_connection_close(transport_connection_t *connection);

#endif //COBRA_TRANSPORT_CONNECTION_H