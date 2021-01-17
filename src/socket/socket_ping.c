#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

void cobra__socket_start_ping(cobra_socket_t *sock) {
    uv_timer_start(&sock->timer_handle,
                   cobra__socket_timer_callback,
                   COBRA_SOCKET_PING_TIMEOUT,
                   COBRA_SOCKET_PING_TIMEOUT);
}

void cobra__socket_ping_refresh_timer(cobra_socket_t *sock) {
    uv_timer_again(&sock->timer_handle);
}

void cobra__socket_ping_frame(cobra_socket_t *sock) {
    if (sock->alive == COBRA_SOCKET_ALIVE_OK) {
        cobra__socket_write(sock, NULL, 0);
        return;
    }

    sock->alive = COBRA_SOCKET_ALIVE_OK;
}

void cobra__socket_timer_callback(uv_timer_t *timer_handle) {
    cobra_socket_t *sock = uv_handle_get_data((uv_handle_t *) timer_handle);

    sock->alive = COBRA_SOCKET_ALIVE_PING_SENT;
    cobra__socket_write(sock, NULL, 0);

    uv_timer_start(&sock->check_timer_handle,
                   cobra__socket_check_timer_callback,
                   COBRA_SOCKET_PING_CHECK_TIMEOUT,
                   0);
}

void cobra__socket_check_timer_callback(uv_timer_t *timer_handle) {
    cobra_socket_t *sock = uv_handle_get_data((uv_handle_t *) timer_handle);

    if (sock->alive != COBRA_SOCKET_ALIVE_OK) {
        cobra__socket_close(sock, COBRA_SOCKET_ERR_UNREACHABLE);
    }
}