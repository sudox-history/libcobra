#define COBRA_DISCOVERY_PRIVATE
#include "discovery.h"

cobra_discovery_t *cobra_discovery_create(found_cb on_found) {
    cobra_discovery_t *discovery = malloc(sizeof(cobra_discovery_t));

    uv_loop_init(&discovery->loop);
    uv_udp_init(&discovery->loop, &discovery->udp_handle);
    uv_timer_init(&discovery->loop, &discovery->timer_handle);

    discovery->loop.data = discovery;
    discovery->udp_handle.data = discovery;
    discovery->timer_handle.data = discovery;

    discovery->buffer = cobra_buffer_create(5);
    discovery->listening = false;
    discovery->scanning = false;

    discovery->on_found = on_found;
    return discovery;
}

void cobra_discovery_destroy(cobra_discovery_t *discovery) {
    cobra_discovery_scan_close(discovery);
    cobra_discovery_listen_close(discovery);
    free(discovery);
}

void discovery_on_multicast_recv(uv_udp_t *handle,
                                 ssize_t read_len,
                                 const uv_buf_t *_,
                                 const struct sockaddr *addr,
                                 unsigned flags) {
    cobra_discovery_t *discovery = handle->data;

    if (read_len <= 0)
        return;

    // Skipping read bytes
    cobra_buffer_skip(discovery->buffer, read_len);

    if (!cobra_buffer_equals(discovery->buffer, COBRA_DISCOVERY_PACKET, 5))
        return;

    uv_buf_t send_buf = {
            .base = (char *) COBRA_DISCOVERY_PACKET,
            .len = 5
    };

    uv_udp_send(&discovery->send_req,
                &discovery->udp_handle,
                &send_buf,
                1,
                addr,
                NULL);

    cobra_buffer_clear(discovery->buffer);
}

void discovery_on_scan_recv(uv_udp_t *handle,
                            ssize_t read_len,
                            const uv_buf_t *_,
                            const struct sockaddr *addr,
                            unsigned flags) {
    cobra_discovery_t *discovery = handle->data;

    if (read_len <= 0)
        return;

    // Skipping read bytes
    cobra_buffer_skip(discovery->buffer, read_len);

    if (!cobra_buffer_equals(discovery->buffer, COBRA_DISCOVERY_PACKET, 5))
        return;

    char host[100];
    uv_ip4_name((const struct sockaddr_in *) addr, host, 100);

    if (discovery->on_found)
        discovery->on_found(discovery, host);

    cobra_buffer_clear(discovery->buffer);
}

void discovery_on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *buf) {
    cobra_discovery_t *discovery = handle->data;

    uint8_t *base;
    int len = cobra_buffer_write_pointer(discovery->buffer, &base);

    buf->base = (char *) base;
    buf->len = len;
}

void discovery_on_timer(uv_timer_t *handle) {
    cobra_discovery_t *discovery = handle->data;

    uv_buf_t send_buf = {
            .base = (char *) COBRA_DISCOVERY_PACKET,
            .len = 5
    };

    struct sockaddr_in addr;
    uv_ip4_addr(COBRA_DISCOVERY_ADDR, COBRA_DISCOVERY_PORT, &addr);

    uv_udp_send(&discovery->send_req,
                &discovery->udp_handle,
                &send_buf,
                1,
                (const struct sockaddr *) &addr,
                NULL);
}

int cobra_discovery_listen(cobra_discovery_t *discovery) {
    struct sockaddr_in addr;
    uv_ip4_addr(COBRA_DISCOVERY_ADDR_ANY, COBRA_DISCOVERY_PORT, &addr);

    if (uv_udp_bind(&discovery->udp_handle,
                    (const struct sockaddr *) &addr,
                    0))
        return COBRA_DISCOVERY_ERR_BINDING;

    if (uv_udp_set_membership(&discovery->udp_handle,
                              COBRA_DISCOVERY_ADDR,
                              NULL,
                              UV_JOIN_GROUP))
        return COBRA_DISCOVERY_ERR_BINDING;

    if (uv_udp_recv_start(&discovery->udp_handle,
                          discovery_on_alloc,
                          discovery_on_multicast_recv))
        return COBRA_DISCOVERY_ERR_LISTENING;

    discovery->listening = true;
    uv_run(&discovery->loop, UV_RUN_DEFAULT);
    return COBRA_DISCOVERY_OK;
}

int cobra_discovery_scan(cobra_discovery_t *discovery) {
    struct sockaddr_in addr;
    uv_ip4_addr(COBRA_DISCOVERY_ADDR_ANY, COBRA_DISCOVERY_PORT, &addr);

    if (uv_udp_bind(&discovery->udp_handle,
                    (const struct sockaddr *) &addr,
                    0))
        return COBRA_DISCOVERY_ERR_BINDING;


    if (uv_udp_recv_start(&discovery->udp_handle,
                          discovery_on_alloc,
                          discovery_on_scan_recv))
        return COBRA_DISCOVERY_ERR_LISTENING;

    uv_timer_start(&discovery->timer_handle,
                   discovery_on_timer,
                   0,
                   COBRA_DISCOVERY_DISCOVER_TIMEOUT);

    discovery->scanning = true;
    uv_run(&discovery->loop, UV_RUN_DEFAULT);
    return COBRA_DISCOVERY_OK;
}

void cobra_discovery_listening_close(cobra_discovery_t *discovery) {
    if (!discovery->listening)
        return;

    uv_udp_recv_stop(&discovery->udp_handle);
}

void cobra_discovery_scan_close(cobra_discovery_t *discovery) {
    if (!discovery->scanning)
        return;

    uv_timer_stop(&discovery->timer_handle);
    uv_udp_recv_stop(&discovery->udp_handle);
}

void cobra_discovery_set_data(cobra_discovery_t *discovery, void *data) {
    discovery->data = data;
}

void *cobra_discovery_get_data(cobra_discovery_t *discovery) {
    return discovery->data;
}