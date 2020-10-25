#include <stdlib.h>

#define COBRA_TCP_CLIENT_PRIVATE
#include "tcp_client.h"

tcp_client_t *tcp_client_create() {
    tcp_client_t *client = malloc(sizeof(tcp_client_t));



    return client;
}