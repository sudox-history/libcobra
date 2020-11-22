#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <uv.h>
#include "cobra.h"

void writer_on_close(cobra_socket_t *socket, int error) {
    printf("WRITER CLOSED: %d\n", error);
}

void writer_on_alloc(cobra_socket_t *socket, uint8_t **data, uint64_t length) {
    *data = malloc(length);
}

int writer_fd;

void writer_on_data(cobra_socket_t *socket, uint8_t *data, uint64_t length) {
    printf("RECEIVED: %llu\n", length);
    write(writer_fd, data, length);
    free(data);
}

void writer_on_connection(cobra_server_t *server, cobra_socket_t *socket) {
    printf("NEW CONNECTION\n");
    cobra_socket_set_callbacks(socket,
                               NULL,
                               writer_on_close,
                               writer_on_alloc,
                               writer_on_data,
                               NULL);
}

void start_writer() {
    cobra_server_t *server = cobra_server_create(10);
    cobra_server_set_callbacks(server,
                               writer_on_connection,
                               NULL);
    cobra_server_listen(server, "127.0.0.1", "5000");
}

int read_fd;

void read_file(cobra_socket_t *socket) {
    printf("READING FILE\n");
    uint8_t data[COBRA_SOCKET_PACKET_BODY_MAX_LENGTH];

    while (true) {
        int read_length = read(read_fd, data, COBRA_SOCKET_PACKET_BODY_MAX_LENGTH);
        if (read_length <= 0) {
            printf("READER ENDED FILE\n");
            close(read_fd);
            break;
        }

        if (cobra_socket_send(socket, data, read_length)
            == COBRA_SOCKET_ERR_QUEUE_FULL) {
            printf("SEND QUEUE FULL, STOPPING READING\n");
            break;
        }
    }
}

void reader_on_connect(cobra_socket_t *socket) {
    printf("READER CONNECTED\n");
    read_file(socket);
}

void reader_on_close(cobra_socket_t *socket, int error) {
    printf("READER CLOSED: %d\n", error);
}

void reader_on_drain(cobra_socket_t *socket) {
    printf("DRAIN CALLED\n");
    read_file(socket);
}

void start_reader() {
    cobra_socket_t *socket = cobra_socket_create(10);
    cobra_socket_set_callbacks(socket,
                               reader_on_connect,
                               reader_on_close,
                               NULL,
                               NULL,
                               reader_on_drain);
    cobra_socket_connect(socket, "127.0.0.1", "5000");
}

int main() {
    writer_fd = open("/User/undefined/Desktop/bigfile_write", O_WRONLY);
    read_fd = open("/User/undefined/Desktop/bigfile_read", O_WRONLY);

    if (writer_fd < 0 || read_fd < 0) {
        printf("FAILED TO OPEN FILES\n");
        return 1;
    }

    uv_thread_t write_thread;
    uv_thread_create(&write_thread, start_writer, NULL);

    uv_thread_t read_thread;
    uv_thread_create(&read_thread, start_reader, NULL);

    uv_thread_join(&write_thread);
    uv_thread_join(&read_thread);
}