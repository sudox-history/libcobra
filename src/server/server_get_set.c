#define COBRA_SERVER_PRIVATE
#include "cobra/server.h"

void cobra_server_set_callbacks(cobra_server_t *server,
                                cobra_server_connection_cb connection_callback,
                                cobra_server_close_cb close_callback) {
    uv_mutex_lock(&server->mutex_handle);
    server->connection_callback = connection_callback;
    server->close_callback = close_callback;
    uv_mutex_unlock(&server->mutex_handle);
}

void cobra_server_set_data(cobra_server_t *server, void *data) {
    uv_mutex_lock(&server->mutex_handle);
    server->data = data;
    uv_mutex_unlock(&server->mutex_handle);
}
void *cobra_server_get_data(cobra_server_t *server) {
    uv_mutex_lock(&server->mutex_handle);
    void *data = server->data;
    uv_mutex_unlock(&server->mutex_handle);

    return data;
}