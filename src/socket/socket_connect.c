#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

cobra_socket_err_t cobra_socket_connect(cobra_socket_t *sock,
                                        char *host,
                                        char *port) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->state != COBRA_SOCKET_STATE_CLOSED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_ALREADY_OPENED;
    }

    sock->state = COBRA_SOCKET_STATE_INITIALIZING;
    sock->alive = COBRA_SOCKET_ALIVE_OK;

    // Necessary to re-init handles each time you want to open connection.
    // Initialization MUST be under mutex protection because after unlock()
    // user can call immediately cobra_socket_close() which deals with loop.
    uv_tcp_init(&sock->loop, &sock->tcp_handle);
    uv_timer_init(&sock->loop, &sock->timer_handle);
    uv_timer_init(&sock->loop, &sock->check_timer_handle);
    cobra_async_bind(&sock->write_async, &sock->loop);
    cobra_async_bind(&sock->close_async, &sock->loop);

    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_connect_ctx_t *connect_ctx =
        malloc(sizeof(cobra__socket_connect_ctx_t));

    char *host_copy = malloc(strlen(host) + 1);
    char *port_copy = malloc(strlen(port) + 1);

    strcpy(host_copy, host);
    strcpy(port_copy, port);

    connect_ctx->sock = sock;
    connect_ctx->host = host_copy;
    connect_ctx->port = port_copy;

    uv_thread_create(&sock->thread_handle, cobra__socket_connect_thread,
                     connect_ctx);

    return COBRA_SOCKET_OK;
}

void cobra__socket_connect_thread(void *data) {
    cobra__socket_connect_ctx_t *connect_ctx = data;

    cobra_socket_t *sock = connect_ctx->sock;
    char *host = connect_ctx->host;
    char *port = connect_ctx->port;

    // Don't forget to free context
    free(connect_ctx);

    uv_mutex_lock(&sock->mutex_handle);

    // During thread creation user could run cobra_socket_close()
    if (sock->state != COBRA_SOCKET_STATE_CLOSING) {
        sock->state = COBRA_SOCKET_STATE_RESOLVING;
        uv_mutex_unlock(&sock->mutex_handle);

        sock->resolve_request = malloc(sizeof(uv_getaddrinfo_t));
        uv_req_set_data((uv_req_t *)sock->resolve_request, sock);

        uv_getaddrinfo(&sock->loop, sock->resolve_request,
                       cobra__socket_resolve_callback, host, port, NULL);

        free(host);
        free(port);
    } else {
        uv_mutex_unlock(&sock->mutex_handle);
    }

    uv_run(&sock->loop, UV_RUN_DEFAULT);
}

void cobra__socket_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo) {
    cobra_socket_t *sock = uv_req_get_data((uv_req_t *)resolve_request);

    free(resolve_request);
    sock->resolve_request = NULL;

    // If user called cobra_socket_socket() during connecting
    if (error == UV_ECANCELED)
        return;

    if (error != 0) {
        cobra__socket_close(sock, COBRA_SOCKET_ERR_RESOLVING);
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);
    sock->state = COBRA_SOCKET_STATE_CONNECTING;
    uv_mutex_unlock(&sock->mutex_handle);

    sock->connect_request = malloc(sizeof(uv_connect_t));
    uv_req_set_data((uv_req_t *)sock->connect_request, sock);

    uv_tcp_connect(sock->connect_request, &sock->tcp_handle, addrinfo->ai_addr,
                   cobra__socket_connect_callback);

    // Specified in libuv docs
    uv_freeaddrinfo(addrinfo);
}

void cobra__socket_connect_callback(uv_connect_t *connect_request, int error) {
    cobra_socket_t *sock = uv_req_get_data((uv_req_t *)connect_request);

    free(connect_request);
    sock->connect_request = NULL;

    // If user called cobra_socket_socket() during connecting
    if (error == UV_ECANCELED)
        return;

    if (error != 0) {
        cobra__socket_close(sock, COBRA_SOCKET_ERR_CONNECTING);
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);
    cobra_socket_connect_cb connect_callback = sock->connect_callback;
    sock->state = COBRA_SOCKET_STATE_CONNECTED;
    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_start_read(sock);
    cobra__socket_start_ping(sock);

    if (connect_callback != NULL)
        connect_callback(sock);
}