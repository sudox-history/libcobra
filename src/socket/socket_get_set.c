#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

void cobra_socket_set_callbacks(cobra_socket_t *sock,
                                cobra_socket_connect_cb connect_callback,
                                cobra_socket_close_cb close_callback,
                                cobra_socket_alloc_cb alloc_callback,
                                cobra_socket_read_cb read_callback,
                                cobra_socket_write_cb write_callback,
                                cobra_socket_drain_cb drain_callback) {
    uv_mutex_lock(&sock->mutex_handle);
    sock->connect_callback = connect_callback;
    sock->close_callback = close_callback;
    sock->alloc_callback = alloc_callback;
    sock->read_callback = read_callback;
    sock->write_callback = write_callback;
    sock->drain_callback = drain_callback;
    uv_mutex_unlock(&sock->mutex_handle);
}

void cobra_socket_set_data(cobra_socket_t *sock, void *data) {
    uv_mutex_lock(&sock->mutex_handle);
    sock->data = data;
    uv_mutex_unlock(&sock->mutex_handle);
}
void *cobra_socket_get_data(cobra_socket_t *sock) {
    uv_mutex_lock(&sock->mutex_handle);
    void *data = sock->data;
    uv_mutex_unlock(&sock->mutex_handle);

    return data;
}