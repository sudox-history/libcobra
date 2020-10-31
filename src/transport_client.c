#include <stdlib.h>
#include <uv.h>

#define COBRA_TRANSPORT_CLIENT_PRIVATE

#include "transport_client.h"
#include "transport_errors.h"

transport_client_t *transport_client_create() {
    transport_client_t *client = malloc(sizeof(transport_client_t));


    return client;
}

void transport_client_destroy(transport_client_t *client) {
    free(client);
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
    transport_client_t *client = resolver->data;

    if (status) {
        client->on_connect(client, COBRA_TRANSPORT_DNS_ERR);
        return;
    }

    client->on_connect(client, COBRA_TRANSPORT_OK);
 }

void transport_client_connect(transport_client_t *client, char *host, char *port) {
    uv_getaddrinfo_t resolver;
    resolver.data = client;

    if (uv_getaddrinfo(client->loop, &resolver, on_resolved, host, port, NULL)) {
        client->on_connect(client, COBRA_TRANSPORT_DNS_ERR);
    }
}