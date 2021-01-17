#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

cobra_socket_t *cobra_socket_create(int write_queue_size) {
    cobra_socket_t *sock = malloc(sizeof(cobra_socket_t));

    uv_loop_init(&sock->loop);
    uv_mutex_init(&sock->mutex_handle);
    cobra_async_init(&sock->write_async, write_queue_size);
    cobra_async_init(&sock->close_async, 1);

    uv_handle_set_data((uv_handle_t *) &sock->tcp_handle, sock);
    uv_handle_set_data((uv_handle_t *) &sock->timer_handle, sock);
    uv_handle_set_data((uv_handle_t *) &sock->check_timer_handle, sock);
    cobra_async_set_data(&sock->write_async, sock);
    cobra_async_set_data(&sock->close_async, sock);

    sock->resolve_request = NULL;
    sock->connect_request = NULL;

    cobra_async_set_callbacks(&sock->write_async,
                              cobra__socket_write_async_send_callback,
                              cobra__socket_write_async_drain_callback,
                              cobra__socket_async_close_callback);

    cobra_async_set_callbacks(&sock->close_async,
                              cobra__socket_close_async_send_callback,
                              NULL,
                              cobra__socket_async_close_callback);

    cobra_buffer_init(&sock->read_buffer, COBRA_SOCKET_FRAME_MAX_LENGTH);
    return sock;
}

cobra_socket_err_t cobra_socket_destroy(cobra_socket_t *sock) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->state != COBRA_SOCKET_STATE_CLOSED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_NOT_CLOSED;
    }

    uv_mutex_unlock(&sock->mutex_handle);
    cobra_async_deinit(&sock->write_async);
    cobra_async_deinit(&sock->close_async);
    cobra_buffer_deinit(&sock->read_buffer);
    free(socket);

    return COBRA_SOCKET_OK;
}

void cobra__socket_bind(cobra_socket_t *sock, uv_loop_t *loop) {
    uv_tcp_init(loop, &sock->tcp_handle);
    uv_timer_init(loop, &sock->timer_handle);
    uv_timer_init(loop, &sock->check_timer_handle);
}

uv_tcp_t *cobra__socket_get_tcp_handle(cobra_socket_t *sock) {
    return &sock->tcp_handle;
}
