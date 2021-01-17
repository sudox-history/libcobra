#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cobra/utils.h"
#include "cobra.h"

void client_on_connect(cobra_socket_t *sock) {
    printf("Client socket: Connect callback invoked\n");

}

void client_on_alloc(cobra_socket_t *sock, uint8_t **data, uint64_t length) {
    printf("Client socket: Alloc callback invoked\n");
    *data = malloc(length);
}

void client_on_read(cobra_socket_t *sock, uint8_t *data, uint64_t length) {
    printf("Client socket: Read callback invoked\n");
    cobra_utils_print_array(data, length);
}

void client_on_close(cobra_socket_t *sock, cobra_socket_err_t error) {
    printf("Client socket: Close callback invoked, error: %d\n", error);
    cobra_socket_destroy(sock);
}

void server_socket_on_alloc(cobra_socket_t *sock, uint8_t **data, uint64_t length) {
    printf("Server socket: Alloc callback invoked\n");
    *data = malloc(length);
}

void server_socket_on_read(cobra_socket_t *sock, uint8_t *data, uint64_t length) {
    printf("Server socket: Read callback invoked\n");
    cobra_utils_print_array(data, length);
}

void server_socket_on_close(cobra_socket_t *sock, cobra_socket_err_t error) {
    printf("Server socket: Close callback invoked, error: %d\n", error);
    cobra_socket_destroy(sock);
}

void server_on_connection(cobra_server_t *server, cobra_socket_t *sock) {
    printf("Server: Connection callback invoked\n");
    cobra_socket_set_callbacks(sock,
                               NULL,
                               server_socket_on_close,
                               server_socket_on_alloc,
                               server_socket_on_read,
                               NULL,
                               NULL);
}

void server_on_close(cobra_server_t *server, cobra_server_err_t error) {
    printf("Server closed, reason: %d", error);
    cobra_server_close(server);
}

void run_server() {
    cobra_server_t *server = cobra_server_create(10);
    cobra_server_set_callbacks(server, server_on_connection, server_on_close);
    cobra_server_listen(server, "127.0.0.1", "5000");
}

void run_socket() {
    cobra_socket_t *sock = cobra_socket_create(10);
    cobra_socket_set_callbacks(sock,
                               client_on_connect,
                               client_on_close,
                               client_on_alloc,
                               client_on_read,
                               NULL,
                               NULL);

    cobra_socket_connect(sock, "127.0.0.1", "5000");
}

int main() {
    run_server();
    sleep(1);
    run_socket();
    sleep(1000000);
}