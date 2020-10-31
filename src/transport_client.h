#ifndef COBRA_TRANSPORT_CLIENT_H
#define COBRA_TRANSPORT_CLIENT_H

typedef struct transport_client_t transport_client_t;

#ifdef COBRA_TRANSPORT_CLIENT_PRIVATE

struct transport_client_t {
    uv_loop_t *loop;

    void (*on_connect)(transport_client_t *client, int result);
};

#endif

transport_client_t *transport_client_create();

void transport_client_destroy(transport_client_t *client);

void transport_client_connect(transport_client_t *client, char *host, char *port);

#endif //COBRA_TRANSPORT_CLIENT_H