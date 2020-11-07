#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "internal/tcp_connection.h"
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

void on_found(cobra_discovery_t *discovery, char *host) {
    printf("FOUND NEW DEVICE: %s\n", host);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    cobra_discovery_t *discovery = cobra_discovery_create(on_found);
    cobra_discovery_scan(discovery);
}
