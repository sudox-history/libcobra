#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

int cobra_socket_connect(cobra_socket_t *sock, char *host, char *port) {
    uv_mutex_lock(&sock->mutex_handle);

	if (sock->connection_status != COBRA_SOCKET_STATUS_CLOSED) {
	    uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_NOT_CLOSED;
    }

    uv_getaddrinfo_t *resolve_request = malloc(sizeof(uv_getaddrinfo_t));
    resolve_request->data = sock;

    uv_getaddrinfo(&sock->loop,
                   resolve_request,
                   cobra__socket_resolve_callback,
                   host,
                   port,
                   NULL);

    uv_mutex_unlock(&sock->mutex_handle);
    uv_run(&sock->loop, UV_RUN_DEFAULT);

    return COBRA_SOCKET_OK;
}

