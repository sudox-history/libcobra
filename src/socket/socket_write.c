#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

cobra_socket_err_t cobra_socket_write(cobra_socket_t *sock, uint8_t *data, uint64_t length) {
    if (length == 0)
        return COBRA_SOCKET_ERR_WRONG_DATA;

    return cobra__socket_write(sock, data, length);
}

cobra_socket_err_t cobra__socket_write(cobra_socket_t *sock, uint8_t *data, uint64_t length) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->state != COBRA_SOCKET_STATE_CONNECTED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_NOT_CONNECTED;
    }

    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_write_ctx_t *write_ctx =
            malloc(sizeof(cobra__socket_write_ctx_t));

    write_ctx->sock = sock;
    write_ctx->data = data;
    write_ctx->length = length;

    // That function is thread safe
    int result = cobra_async_send(&sock->write_async, write_ctx);

    // Make map function
    switch (result) {
        case COBRA_ASYNC_OK:
            return COBRA_SOCKET_OK;

        case COBRA_ASYNC_ERR_QUEUE_FULL:
            return COBRA_SOCKET_ERR_QUEUE_FULL;

        case COBRA_ASYNC_ERR_QUEUE_OVERFLOW:
            return COBRA_SOCKET_ERR_QUEUE_OVERFLOW;

        default:
            return COBRA_SOCKET_OK;
    }
}

void cobra__socket_write_async_send_callback(cobra_async_t *async, void *write_ctx_any) {
    cobra__socket_write_ctx_t *write_ctx = write_ctx_any;

    cobra_socket_t *sock = write_ctx->sock;
    uint8_t *data = write_ctx->data;
    uint64_t length = write_ctx->length;

    // Don't forget to free context
    free(write_ctx);

    uv_mutex_lock(&sock->mutex_handle);
    cobra_socket_write_cb write_callback = sock->write_callback;

    if (sock->state == COBRA_SOCKET_STATE_CLOSING) {
        uv_mutex_unlock(&sock->mutex_handle);

        if (write_callback != NULL)
            write_callback(sock, data, length, COBRA_SOCKET_ERR_NOT_CONNECTED);

        return;
    }

    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_uv_write_ctx_t *uv_write_context
            = malloc(sizeof(cobra__socket_uv_write_ctx_t));

    uv_write_context->sock = sock;
    uv_write_context->data = data;
    uv_write_context->length = length;

    cobra_buffer_init(&uv_write_context->frame, length + COBRA_SOCKET_FRAME_HEADER_LENGTH);
    uv_req_set_data((uv_req_t *) &uv_write_context->request, uv_write_context);

    cobra_buffer_write_uint(&uv_write_context->frame, length, COBRA_SOCKET_FRAME_HEADER_LENGTH);
    cobra_buffer_write(&uv_write_context->frame, data, length);

    uv_buf_t write_buffer = {
            .base = (char *) cobra_buffer_read_pointer(&uv_write_context->frame),
            .len = length + COBRA_SOCKET_FRAME_HEADER_LENGTH
    };

    uv_write(&uv_write_context->request,
             (uv_stream_t *) &sock->tcp_handle,
             &write_buffer,
             1,
             cobra__socket_write_callback);
}

void cobra__socket_write_async_drain_callback(cobra_async_t *async) {
    cobra_socket_t *sock = cobra_async_get_data(async);

    uv_mutex_lock(&sock->mutex_handle);
    cobra_socket_drain_cb drain_callback = sock->drain_callback;
    uv_mutex_unlock(&sock->mutex_handle);

    if (drain_callback)
        drain_callback(sock);
}

void cobra__socket_write_callback(uv_write_t *write_request, int error) {
    cobra__socket_uv_write_ctx_t *uv_write_ctx = uv_req_get_data((uv_req_t *) write_request);

    cobra_socket_t *sock = uv_write_ctx->sock;
    uint8_t *data = uv_write_ctx->data;
    uint64_t length = uv_write_ctx->length;

    // Don't forget to free context
    cobra_buffer_deinit(&uv_write_ctx->frame);
    free(uv_write_ctx);

    if (error != 0) {
        cobra__socket_close(sock, COBRA_SOCKET_ERR_UNKNOWN_WRITE_ERROR);
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);
    cobra_socket_write_cb write_callback = sock->write_callback;
    uv_mutex_unlock(&sock->mutex_handle);

    if (write_callback)
        write_callback(sock, data, length, COBRA_SOCKET_OK);
}

