//#include "internal/buffer.h"
//#include <string.h>
//#include <stdlib.h>
//#include <uv.h>
//
//#define COBRA_TRANSPORT_CONNECTION_PRIVATE
//
//#include "transport_connection.h"
//#include "transport_constants.h"
//
//transport_connection_t *transport_connection_create(
//        transport_client_connect_cb on_connect,
//        transport_client_close_cb on_close,
//        transport_client_error_cb on_error,
//        transport_client_packet_cb on_packet
//) {
//    transport_connection_t *connection = malloc(sizeof(transport_connection_t));
//    connection->current_packet_len = 0;
//
//    // Setting callbacks
//    connection->on_connect = on_connect;
//    connection->on_close = on_close;
//    connection->on_error = on_error;
//    connection->on_packet = on_packet;
//
//    // Initializing libuv loop
//    uv_loop_init(&connection->loop);
//    connection->loop.data = connection;
//    connection->buffer = transport_buffer_create(65535);
//
//    // Initializing libuv tcp handle
//    uv_tcp_init(&connection->loop, &connection->tcp_handle);
//    connection->tcp_handle.data = connection;
//
//    // Initializing libuv tcp connect req
//    connection->tcp_connect_req.data = connection;
//
//    return connection;
//}
//
//void transport_connection_destroy(transport_connection_t *client) {
//    free(client);
//}
//
//void on_read(uv_stream_t *stream, ssize_t len, const uv_buf_t *buf) {
//    transport_connection_t *connection = stream->data;
//
//    while (1) {
//        if (connection->current_packet_len == 0) {
//            if (transport_buffer_position(connection->buffer) < COBRA_TRANSPORT_PACKET_HEADER_LEN) {
//                transport_buffer_write(connection->buffer, buf->base, len);
//                break;
//            }
//
//            connection->current_packet_len = transport_buffer_read_number(connection->buffer,
//                                                                          COBRA_TRANSPORT_PACKET_HEADER_LEN);
//            len -= connection->current_packet_len;
//        }
//
//        if (connection->current_packet_len <= transport_buffer_position(connection->buffer)) {
//            unsigned char *data = malloc(connection->current_packet_len);
//            memcpy(data, &connection->buffer->data[COBRA_TRANSPORT_PACKET_HEADER_LEN], connection->current_packet_len);
//            transport_buffer_clear(connection->buffer);
//
//            len -= connection->current_packet_len;
//            connection->on_packet(connection, data, connection->current_packet_len);
//            connection->current_packet_len = 0;
//        } else {
//            transport_buffer_write(connection->buffer, buf->base, len);
//            break;
//        }
//    }
//
//    free(buf->base);
//}
//
//void on_alloc(uv_handle_t *handle, size_t suggested_len, uv_buf_t *buf) {
//    buf->base = malloc(65535);
//    buf->len = 65535;
//}
//
//void on_connected(uv_connect_t *connect_req, int status) {
//    transport_connection_t *connection = connect_req->data;
//
//    if (status) {
//        if (connection->on_error) {
//            connection->on_error(connection, COBRA_TRANSPORT_CONNECTION_ERR);
//        }
//
//        return;
//    }
//
//    if (connection->on_connect) {
//        connection->on_connect(connection);
//    }
//
//    uv_read_start((uv_stream_t *) &connection->tcp_handle, on_alloc, on_read);
//}
//
//void on_resolved(uv_getaddrinfo_t *getaddr_req, int status, struct addrinfo *ai) {
//    transport_connection_t *connection = getaddr_req->data;
//
//    if (status) {
//        if (connection->on_error) {
//            connection->on_error(connection, COBRA_TRANSPORT_CONNECTION_ERR);
//        }
//
//        return;
//    }
//
//    // Trying to connect
//    uv_tcp_connect(&connection->tcp_connect_req,
//                   &connection->tcp_handle,
//                   ai->ai_addr,
//                   on_connected);
//
//    uv_freeaddrinfo(ai);
//}
//
//void transport_connection_connect(transport_connection_t *connection, char *host, char *port) {
//    uv_getaddrinfo_t getaddr_req = {
//            .data = connection
//    };
//
//    // In async mode function always returns 0
//    uv_getaddrinfo(&connection->loop,
//                   &getaddr_req,
//                   on_resolved,
//                   host,
//                   port,
//                   NULL);
//
//    uv_run(&connection->loop, UV_RUN_DEFAULT);
//}