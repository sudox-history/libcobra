#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "internal/tcp_connection.h"
#include "internal/tcp_server.h"
#include "internal/discovery.h"

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

void on_connection(cobra_tcp_server_t *server, cobra_tcp_connection_t *connection) {
    printf("NEW CONNECTION\n");
    cobra_tcp_connection_set_callbacks(connection, NULL, on_closed, on_data, NULL);
}

void listen_test() {
    cobra_tcp_server_t *server = cobra_server_create();
    cobra_tcp_server_set_callbacks(server, on_connection, NULL);
    cobra_tcp_server_listen(server, "0.0.0.0", 5000);
}

void on_connected1(cobra_tcp_connection_t *connection) {
    uint8_t buf[10];
    for (int i = 0; i < 2; i++) {
        printf("SENDING %d\n", i);
        cobra_tcp_connection_send(connection, buf, 10);
    }

    cobra_tcp_connection_close(connection);
}

void on_found(cobra_discovery_t *discovery, char *host) {
    printf("FOUND\n");
    cobra_tcp_connection_t *connection = cobra_tcp_connection_create();
    cobra_tcp_connection_set_callbacks(connection, on_connected1, NULL, NULL, NULL);
    cobra_tcp_connection_connect(connection, host, "5000");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    //uv_thread_t thread_id;
    //uv_thread_create(&thread_id, listen_test, 0);

    cobra_discovery_t *discovery = cobra_discovery_create();
    cobra_discovery_set_callbacks(discovery, on_found);
    cobra_discovery_scan(discovery);
}
