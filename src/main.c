#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <unistd.h>

#define COBRA_TCP_CONNECTION_PRIVATE
#include "internal/tcp_connection.h"
#include "internal/tcp_server.h"
#include "internal/discovery.h"

int fd;

uint8_t zeros[65535];

void local_on_data(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len) {
    static int counter = 0;
    counter++;
    printf("NEW DATA: %d\n", len);

//    if (memcmp(data, zeros, len) != 0) {
//        printf("FAILURE ON PACKET #%d\n", counter);
//    }

    write(fd, data, len);
    free(data);
}

void on_connection_close(cobra_tcp_connection_t *connection, int status) {
    close(fd);
}

void local_on_connection(cobra_tcp_server_t *server, cobra_tcp_connection_t *connection) {
    printf("NEW CONNECTION\n");
    cobra_tcp_connection_set_callbacks(connection, NULL, on_connection_close, local_on_data, NULL);
}

void local_on_close(cobra_tcp_server_t *server, int status) {
    printf("SERVER CLOSED: %d\n", status);
}

void run_server() {
    cobra_tcp_server_t *server = cobra_server_create();
    cobra_tcp_server_set_callbacks(server, local_on_connection, local_on_close);
    int result = cobra_tcp_server_listen(server, "127.0.0.1", 5000);

    printf("Server stopped: %d\n", result);
}

void on_connected_abccc(cobra_tcp_connection_t *connection) {
    int fd1 = open("/home/undef/kek.pdf", O_RDWR);
    uint8_t data[65535];

    while (true) {
        int read_size = read(fd1, data, 50000);

        printf("READ DATA: %d\n", read_size);
        if (read_size <= 0) {
            printf("FILE READ\n");
            break;
        }

        while (cobra_tcp_connection_send(connection, data, read_size) != 0) {
            printf("ERROR sending\n");
            sleep(1);
        }
        //write(fd, data, read_size);
    }
}

void read_file() {
    cobra_tcp_connection_t *connection = cobra_tcp_connection_create();
    cobra_tcp_connection_set_callbacks(connection, on_connected_abccc, NULL, NULL, NULL);
    cobra_tcp_connection_connect(connection, "127.0.0.1", "5000");
}

void sum(uint8_t *data, int len) {

}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    fd = open("/home/undef/kek1.pdf", O_RDWR);

    uv_thread_t thread;
    uv_thread_create(&thread, run_server, 0);

    uv_thread_t thread2;
    uv_thread_create(&thread, read_file, 0);

    cobra_discovery_t *discovery = cobra_discovery_create();
    cobra_discovery_listen(discovery);
}

//void abc123_on_data(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len) {
//    printf("NEW DATA: %d\n", len);
//    for (int i = 0; i < len; i++) {
//        printf("%.2X ", data[i]);
//    }
//
//    printf("\n");
//}
//
//int main() {
//    setvbuf(stdout, NULL, _IONBF, 0);
//
//    cobra_tcp_connection_t *connection = cobra_tcp_connection_create();
//    cobra_tcp_connection_set_callbacks(connection, NULL, NULL, abc123_on_data, NULL);
//    cobra_tcp_connection_connect(connection, "127.0.0.1", "5001");
//}