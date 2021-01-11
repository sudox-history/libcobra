#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

int cobra_socket_close(cobra_socket_t *sock) {
    return cobra__socket_close(sock, COBRA_SOCKET_OK);
}

int cobra__socket_close(cobra_socket_t *sock, cobra_socket_err_t error) {
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

    cobra__socket_close_ctx_t *close_ctx
            = malloc(sizeof(cobra__socket_close_ctx_t));

    close_ctx->sock = sock;
    close_ctx->error = error;

    // That function is thread safe
    cobra_async_send(&sock->close_async, close_ctx);

    return COBRA_SOCKET_OK;
}

void cobra__socket_close_async_send_callback(cobra_async_t *async, void *data) {
    cobra__socket_close_ctx_t *close_ctx = data;

    cobra_socket_t *sock = close_ctx->sock;
    cobra_socket_err_t error = close_ctx->error;

    // Don't forget to free context
    free(close_ctx);

    // Cancel requests
    if (sock->resolve_request != NULL)
        uv_cancel((uv_req_t *) sock->resolve_request);

    if (sock->connect_request != NULL)
        uv_cancel((uv_req_t *) sock->connect_request);

    // Close handlers and async structures.
    // It's safe to close them here because only one callback can be executed at time
    uv_close((uv_handle_t *) &sock->tcp_handle, cobra__socket_close_callback);
    uv_close((uv_handle_t *) &sock->timer_handle, cobra__socket_close_callback);
    cobra_async_close(&sock->write_async);
    cobra_async_close(&sock->close_async);

    sock->close_error = error;
    sock->closed_handlers_count = 0;
}

static void cobra__socket_close_common_callback(cobra_socket_t *sock) {
    sock->closed_handlers_count++;
    if (sock->closed_handlers_count == COBRA_SOCKET_TOTAL_HANDLERS_COUNT) {
        uv_mutex_lock(&sock->mutex_handle);
        cobra_socket_close_cb close_callback = sock->close_callback;
        uv_mutex_unlock(&sock->mutex_handle);

        if (close_callback != NULL)
            close_callback(sock, sock->close_error);
    }
}

void cobra__socket_close_callback(uv_handle_t *handle) {
    cobra__socket_close_common_callback(uv_handle_get_data(handle));
}

void cobra__socket_close_async_close_callback(cobra_async_t *async) {
    cobra__socket_close_common_callback(cobra_async_get_data(async));
}