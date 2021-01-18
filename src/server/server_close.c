#define COBRA_SERVER_PRIVATE
#include "cobra/server.h"

cobra_server_err_t cobra_server_close(cobra_server_t *server) {
    return cobra__server_close(server, COBRA_SERVER_OK);
}

cobra_server_err_t cobra__server_close(cobra_server_t *server,
                                       cobra_server_err_t error) {
    uv_mutex_lock(&server->mutex_handle);

    if (server->state == COBRA_SERVER_STATE_CLOSED) {
        uv_mutex_unlock(&server->mutex_handle);
        return COBRA_SERVER_ERR_ALREADY_CLOSED;
    }

    // If the socket is already closing - do nothing
    if (server->state == COBRA_SERVER_STATE_CLOSING) {
        uv_mutex_unlock(&server->mutex_handle);
        return COBRA_SERVER_OK;
    }

    server->state = COBRA_SERVER_STATE_CLOSING;
    uv_mutex_unlock(&server->mutex_handle);

    cobra__server_close_ctx_t *close_ctx =
        malloc(sizeof(cobra__server_close_ctx_t));

    close_ctx->server = server;
    close_ctx->error = error;

    // That function is thread safe
    cobra_async_send(&server->close_async, close_ctx);

    return COBRA_SERVER_OK;
}

void cobra__server_close_async_send_callback(cobra_async_t *async, void *data) {
    cobra__server_close_ctx_t *close_ctx = data;

    cobra_server_t *server = close_ctx->server;
    cobra_server_err_t error = close_ctx->error;

    // Don't forget to free context
    free(close_ctx);

    // Cancel requests
    if (server->resolve_request != NULL)
        uv_cancel((uv_req_t *)server->resolve_request);

    // Close handlers and async structures.
    // It's safe to close them here because only one callback can be executed at
    // time
    uv_close((uv_handle_t *)&server->tcp_handle, cobra__server_close_callback);
    cobra_async_close(&server->close_async);

    server->close_error = error;
    server->closed_handlers_count = 0;
}

static void cobra__server_common_close_callback(cobra_server_t *server) {
    server->closed_handlers_count++;

    if (server->closed_handlers_count == COBRA_SERVER_TOTAL_HANDLERS_COUNT) {
        uv_mutex_lock(&server->mutex_handle);
        cobra_server_close_cb close_callback = server->close_callback;
        uv_mutex_unlock(&server->mutex_handle);

        if (close_callback != NULL)
            close_callback(server, server->close_error);
    }
}

void cobra__server_close_callback(uv_handle_t *handle) {
    cobra__server_common_close_callback(uv_handle_get_data(handle));
}

void cobra__server_async_close_callback(cobra_async_t *async) {
    cobra__server_common_close_callback(cobra_async_get_data(async));
}
