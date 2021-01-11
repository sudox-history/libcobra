#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

int cobra_socket_close(cobra_socket_t *sock) {
    return cobra__socket_close(sock, COBRA_SOCKET_OK);
}

int cobra__socket_close(cobra_socket_t *sock, int error) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->state == COBRA_SOCKET_STATE_CLOSED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_ALREADY_CLOSED;
    }

    // If the socket is already closing - do nothing
    if (sock->state == COBRA_SOCKET_STATE_CLOSING) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_OK;
    }

    sock->state = COBRA_SOCKET_STATE_CLOSING;
    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_close_ctx_t *close_context
            = malloc(sizeof(cobra__socket_close_ctx_t));

    close_context->sock = sock;
    close_context->error = error;

    cobra_async_send(&sock->close_async, close_context);
    return COBRA_SOCKET_OK;
}

void cobra__socket_close_async_main_callback(cobra_async_t *async, void *data) {

}