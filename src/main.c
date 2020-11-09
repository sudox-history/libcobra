#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "internal/tcp_connection.h"
#include "internal/tcp_server.h"
#include "internal/discovery.h"

void local_on_data(cobra_tcp_connection_t *connection, uint8_t *data, int len) {
    printf("CLOSING SERVER\n");
    cobra_tcp_server_t *server = cobra_tcp_connection_get_data(connection);
    cobra_tcp_server_close(server);
}

void local_on_connection(cobra_tcp_server_t *server, cobra_tcp_connection_t *connection) {
    printf("NEW CONNECTION\n");
    cobra_tcp_connection_set_data(connection, server);
    cobra_tcp_connection_set_callbacks(connection, NULL, NULL, local_on_data, NULL);
}

void local_on_close(cobra_tcp_server_t *server, int status) {
    printf("SERVER CLOSED: %d\n", status);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    cobra_tcp_server_t *server = cobra_server_create();
    cobra_tcp_server_set_callbacks(server, local_on_connection, local_on_close);
    cobra_tcp_server_listen(server, "0.0.0.0", 5000);
}
