#ifndef COBRA_TRANSPORT_CONNECTION_H
#define COBRA_TRANSPORT_CONNECTION_H

typedef struct transport_connection_t transport_connection_t;

#ifdef COBRA_TRANSPORT_CLIENT_PRIVATE

struct transport_connection_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;
    uv_connect_t tcp_connect_req;

    void (*on_connect)(transport_connection_t *connection);

    void (*on_close)(transport_connection_t *connection);

    void (*on_error)(transport_connection_t *connection, int code);

    void (*on_packet)(transport_connection_t *connection, unsigned char *data, int len);
};

#endif

transport_connection_t *transport_connection_create();

void transport_connection_destroy(transport_connection_t *connection);

void transport_connection_connect(transport_connection_t *connection, char *host, char *port);

int transport_connection_send(transport_connection_t *connection, unsigned char *data, int len);

int transport_connection_close(transport_connection_t *connection);

#endif //COBRA_TRANSPORT_CONNECTION_H