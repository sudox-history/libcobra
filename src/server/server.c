#define COBRA_SERVER_PRIVATE
#include "cobra/server.h"

cobra_server_t *cobra_server_create(int sockets_write_queue_size) {
    cobra_server_t *server = malloc(sizeof(cobra_server_t));

    uv_loop_init(&server->loop);
    uv_mutex_init(&server->mutex_handle);
    cobra_async_init(&server->close_async, 1);

    uv_handle_set_data((uv_handle_t *) &server->tcp_handle, server);
    cobra_async_set_data(&server->close_async, server);

    server->state = COBRA_SERVER_STATE_CLOSED;
    server->resolve_request = NULL;
    server->sockets_write_queue_size = sockets_write_queue_size;
    server->closed_handlers_count = 0;

    cobra_async_set_callbacks(&server->close_async,
                              cobra__server_close_async_send_callback,
                              NULL,
                              cobra__server_async_close_callback);

    return server;
}

// TODO: You can call that method only if you sure that all connections are closed. Otherwise, UB will happen
cobra_server_err_t cobra_server_destroy(cobra_server_t *server) {
    uv_mutex_lock(&server->mutex_handle);

    if (server->state != COBRA_SERVER_STATE_CLOSED) {
        uv_mutex_unlock(&server->mutex_handle);
        return COBRA_SERVER_ERR_NOT_CLOSED;
    }

    uv_mutex_unlock(&server->mutex_handle);
    cobra_async_deinit(&server->close_async);
    free(server);

    return COBRA_SERVER_OK;
}