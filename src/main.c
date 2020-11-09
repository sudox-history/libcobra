#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <unistd.h>

#define COBRA_TCP_CONNECTION_PRIVATE
#include "internal/tcp_connection.h"
#include "internal/tcp_server.h"
#include "internal/discovery.h"

int fd;
int count;
int packets;

void local_on_data(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len) {
    printf("NEW DATA: %d\n", len);

    bool flag = false;
    for (int i = 0; i < len; i++) {
        if (data[i] == 0) {
            count++;

            if (!flag) {
                packets++;
                flag = true;
            }
        }
    }

    write(fd, data, len);
    free(data);
}

void on_connection_close(cobra_tcp_connection_t *connection, int status) {
    printf("Counter: %d\n", count);
    printf("Packets: %d\n", packets);
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
    cobra_tcp_server_listen(server, "0.0.0.0", 5000);
}

void on_connected_abccc(cobra_tcp_connection_t *connection) {
    int fd1 = open("/Users/undefined/Desktop/Kim_Kritskov.pdf", O_RDWR);
    uint8_t data[50000];

    while (true) {
        int read_size = read(fd1, data, 50000);

        printf("READ DATA: %d\n", read_size);
        if (read_size <= 0) {
            printf("ERRROR: %s\n", strerror(errno));
            break;
        }

        while (cobra_tcp_connection_send(connection, data, read_size)) {
            sleep(1);
        }
        //write(fd, data, read_size);
    }

    cobra_tcp_connection_close(connection);
}

void read_file() {
    cobra_tcp_connection_t *connection = cobra_tcp_connection_create();
    cobra_tcp_connection_set_callbacks(connection, on_connected_abccc, NULL, NULL, NULL);
    cobra_tcp_connection_connect(connection, "127.0.0.1", "5000");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    fd = open("/Users/undefined/Desktop/text.pdf", O_CREAT | O_RDWR);
    printf("TEST: %d %s\n", fd, strerror(errno));

    count = 0;
    packets = 0;
    uv_thread_t thread;
    uv_thread_create(&thread, run_server, 0);

    uv_thread_t thread2;
    uv_thread_create(&thread, read_file, 0);

    cobra_discovery_t *discovery = cobra_discovery_create();
    cobra_discovery_listen(discovery);
}

//int main() {
//    cobra_buffer_t *buffer = cobra_buffer_create(10);
//    uint8_t data[5] = {1, 2, 3, 4, 5};
//
//    cobra_buffer_write(buffer, data, 5);
//
//    uint8_t data2[2];
//    cobra_buffer_read(buffer, data2, 2);
//
//    cobra_buffer_fragment(buffer);
//
//    uint8_t data3[3];
//    cobra_buffer_read(buffer, data3, 3);
//
//    int res = 0;
//}