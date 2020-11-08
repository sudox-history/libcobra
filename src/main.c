#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "internal/tcp_connection.h"
#include "internal/tcp_server.h"
#include "internal/discovery.h"

void on_connect(cobra_tcp_connection_t *connection) {
    printf("CONNECTED\n");
}

void on_closed(cobra_tcp_connection_t *connection, int status) {
    printf("CLOSED: %d\n", status);
}

void on_data(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len) {
    printf("NEW DATA:\n");

    for (int i = 0; i < len; i++) {
        printf("%d ", data[i]);
    }

    free(data);
    printf("\n");

    if (len == 5)
        cobra_tcp_connection_close(connection);
}

void on_activity(cobra_tcp_connection_t *connection) {
    printf("ACTIVITY\n");
}

void on_connection(cobra_tcp_server_t *server, cobra_tcp_connection_t *connection) {
    printf("NEW CONNECTION\n");

    cobra_tcp_connection_set_callbacks(connection, on_connect, on_closed, on_data, on_activity);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    cobra_tcp_server_t *server = cobra_server_create();
    cobra_tcp_server_set_callbacks(server, on_connection, NULL);

    cobra_tcp_server_listen(server, "127.0.0.1", 5000);
}
