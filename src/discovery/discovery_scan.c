#define COBRA_DISCOVERY_PRIVATE
#include "cobra/discovery.h"

cobra_discovery_err_t cobra_discovery_scan(cobra_discovery_t *discovery) {
    uv_mutex_lock(&discovery->mutex_handle);

    if (discovery->state != COBRA_DISCOVERY_STATE_CLOSED) {
        uv_mutex_unlock(&discovery->mutex_handle);
        return COBRA_DISCOVERY_ERR_ALREADY_OPENED;
    }

    discovery->state = COBRA_DISCOVERY_STATE_INITIALIZING;

    // Necessary to re-init handles each time you want to open connection.
    // Initialization MUST be under mutex protection because after unlock()
    // user can call immediately cobra_discovery_close() which deals with loop.
    uv_udp_init(&discovery->loop, &discovery->udp_handle);
    uv_timer_init(&discovery->loop, &discovery->timer_handle);
    cobra_async_bind(&discovery->close_async, &discovery->loop);
    uv_mutex_unlock(&discovery->mutex_handle);

    cobra__discovery_scan_ctx_t *scan_ctx =
        malloc(sizeof(cobra__discovery_scan_ctx_t));
    scan_ctx->discovery = discovery;

    uv_thread_create(&discovery->thread_handle, cobra__discovery_scan_thread,
                     scan_ctx);

    return COBRA_DISCOVERY_OK;
}

void cobra__discovery_scan_thread(void *data) {
    cobra__discovery_listen_ctx_t *listen_ctx = data;
    cobra_discovery_t *discovery = listen_ctx->discovery;

    // Don't forget to free context
    free(listen_ctx);

    uv_mutex_lock(&discovery->mutex_handle);

    // During thread creation user could run cobra_discovery_close()
    if (discovery->state != COBRA_DISCOVERY_STATE_CLOSING) {
        discovery->state = COBRA_DISCOVERY_STATE_SCANNING;
        uv_mutex_unlock(&discovery->mutex_handle);

        struct sockaddr_in bind_addr;
        uv_ip4_addr(COBRA_DISCOVERY_ANY_ADDR, COBRA_DISCOVERY_PORT, &bind_addr);

        if (uv_udp_bind(&discovery->udp_handle,
                        (const struct sockaddr *)&bind_addr, 0))
            cobra__discovery_close(discovery, COBRA_DISCOVERY_ERR_BINDING);

        uv_timer_start(&discovery->timer_handle,
                       cobra__discovery_scanner_timer_callback, 0,
                       COBRA_DISCOVERY_TIMEOUT);

        uv_udp_recv_start(&discovery->udp_handle,
                          cobra__discovery_listener_alloc_callback,
                          cobra__discovery_scanner_read_callback);
    } else {
        uv_mutex_unlock(&discovery->mutex_handle);
    }

    uv_run(&discovery->loop, UV_RUN_DEFAULT);
}

void cobra__discovery_scanner_timer_callback(uv_timer_t *timer_handle) {
    cobra_discovery_t *discovery = timer_handle->data;

    struct sockaddr_in addr;
    uv_ip4_addr(COBRA_DISCOVERY_MULTICAST_ADDR, COBRA_DISCOVERY_PORT, &addr);

    uint8_t discovery_packet[COBRA_DISCOVERY_PACKET_SIZE] =
        COBRA_DISCOVERY_PACKET;
    uv_buf_t send_buffer = {.base = (char *)discovery_packet, .len = 5};

    uv_udp_send_t *send_request = malloc(sizeof(uv_udp_send_t));
    uv_req_set_data((uv_req_t *)send_request, discovery);

    uv_udp_send(send_request, &discovery->udp_handle, &send_buffer, 1,
                (const struct sockaddr *)&addr,
                cobra__discovery_listener_send_callback);
}

void cobra__discovery_scanner_read_callback(uv_udp_t *udp_handle,
                                            ssize_t length,
                                            const uv_buf_t *buffer,
                                            const struct sockaddr *addr,
                                            unsigned _) {
    cobra_discovery_t *discovery = udp_handle->data;

    if (length <= 0)
        return;

    // Skipping wrote bytes
    cobra_buffer_write_void(&discovery->read_buffer, length);

    uint8_t discovery_packet[COBRA_DISCOVERY_PACKET_SIZE] =
        COBRA_DISCOVERY_PACKET;
    if (!cobra_buffer_equals(&discovery->read_buffer, discovery_packet,
                             COBRA_DISCOVERY_PACKET_SIZE))
        return;

    char host[COBRA_DISCOVERY_HOST_STRLEN];
    uv_ip4_name((const struct sockaddr_in *)addr, host,
                COBRA_DISCOVERY_HOST_STRLEN);

    uv_mutex_lock(&discovery->mutex_handle);
    cobra_discovery_found_cb found_callback = discovery->found_callback;
    uv_mutex_unlock(&discovery->mutex_handle);

    if (found_callback != NULL)
        found_callback(discovery, host);

    cobra_buffer_clear(&discovery->read_buffer);
}