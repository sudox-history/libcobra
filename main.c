#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cobra/utils.h"
#include "cobra.h"

int send_fd = 0;
int recv_fd = 0;

static void send_file(cobra_socket_t *sock) {
    while (true) {
        uint8_t *buf = malloc(65000);
        int len = read(send_fd, buf, 65000);

        if (len <= 0) {
            free(buf);
            break;
        }

        printf("Client socket: sending data\n");
        int res = cobra_socket_write(sock, buf, len);
        if (res == COBRA_SOCKET_ERR_QUEUE_FULL)
            break;
    }
}

void client_on_connect(cobra_socket_t *sock) {
    printf("Client socket: Connect callback invoked\n");
    send_fd = open("/home/undefined/test-files/test_send.txt", O_RDONLY);
    send_file(sock);
}
void client_on_drain(cobra_socket_t *sock) {
    printf("Client socket: Drain callback invoked\n");
    send_file(sock);
}

void client_on_write(cobra_socket_t *sock, uint8_t *data, uint64_t length, cobra_socket_err_t error) {
    if (length != 0) {
        printf("Client socket: Write callback invoked with error code: %d\n", error);
        free(data);
    }
}

void client_on_close(cobra_socket_t *sock, cobra_socket_err_t error) {
    printf("Client socket: Close callback invoked, error: %d\n", error);
    cobra_socket_destroy(sock);
}

void server_socket_on_alloc(cobra_socket_t *sock, uint8_t **data, uint64_t length) {
    printf("Server socket: Alloc callback invoked with len: %lu\n", length);
    *data = malloc(length);
}

void server_socket_on_read(cobra_socket_t *sock, uint8_t *data, uint64_t length) {
    printf("Server socket: Read callback invoked\n");

    write(recv_fd, data, length);
    free(data);
}

void server_socket_on_close(cobra_socket_t *sock, cobra_socket_err_t error) {
    printf("Server socket: Close callback invoked, error: %d\n", error);
    cobra_socket_destroy(sock);
}

void server_on_connection(cobra_server_t *server, cobra_socket_t *sock) {
    printf("Server: Connection callback invoked\n");
    recv_fd = open("/home/undefined/test-files/test_recv.txt", O_WRONLY);

    cobra_socket_set_callbacks(sock,
                               NULL,
                               server_socket_on_close,
                               server_socket_on_alloc,
                               server_socket_on_read,
                               NULL,
                               NULL);
}

void server_on_close(cobra_server_t *server, cobra_server_err_t error) {
    printf("Server closed, reason: %d\n", error);
}

int main() {
    cobra_server_t *server = cobra_server_create(10);
    cobra_server_set_callbacks(server, server_on_connection, server_on_close);
    cobra_server_listen(server, "127.0.0.1", "5000");

    sleep(1);

    cobra_socket_t *sock = cobra_socket_create(10);
    cobra_socket_set_callbacks(sock,
                               client_on_connect,
                               client_on_close,
                               NULL,
                               NULL,
                               client_on_write,
                               client_on_drain);

    cobra_socket_connect(sock, "127.0.0.1", "5000");

    sleep(1);
    cobra_server_close(server);
    scanf("%s");
}