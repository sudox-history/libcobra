#include <stdio.h>
#include "transport_connection.h"

//void on_connect(transport_connection_t *connection) {
//    printf("CONNECTED!");
//}
//
//void on_close(transport_connection_t *connection) {
//    printf("CLOSED!");
//}
//
//void on_error(transport_connection_t *connection, int code) {
//    printf("ERROR!");
//}
//
//void on_packet(transport_connection_t *connection, unsigned char *data, int len) {
//    printf("PACKET!");
//}
//
//int main() {
//    transport_connection_t *connection = transport_connection_create(on_connect, on_close, on_error, on_packet);
//    transport_connection_connect(connection, "localhost", "6001");
//}

#include <stdbool.h>
#include "internal/buffer.h"

int main() {
    cobra_buffer_t *buffer = cobra_buffer_create(100);
    cobra_buffer_write_uint16(buffer, 256);

    cobra_buffer_resize(buffer, 200);

    int capacity = 0;
    uint8_t *data;
    cobra_buffer_write_pointer(buffer, &data, &capacity);

    cobra_buffer_destroy(buffer);
    printf("test");
}
