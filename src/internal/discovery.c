#define COBRA_DISCOVERY_PRIVATE
#include "discovery.h"

cobra_discovery_t *cobra_discovery_create(found_cb on_found) {
    cobra_discovery_t *discovery = malloc(sizeof(cobra_discovery_t));

    uv_loop_init(&discovery->loop);
    uv_udp_init(&discovery->loop, &discovery->udp_handle);

    discovery->loop.data = discovery;
    discovery->udp_handle.data = discovery;

    discovery->buffer = cobra_buffer_create(5);

    discovery->on_found = on_found;
    return discovery;
}

void cobra_discovery_destroy(cobra_discovery_t *discovery) {
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

void discovery_on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *buf) {
    cobra_discovery_t *discovery = handle->data;

    uint8_t *base;
    int len = cobra_buffer_write_pointer(discovery->buffer, &base);

    buf->base = (char *) base;
    buf->len = len;
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

    uv_run(&discovery->loop, UV_RUN_DEFAULT);
    return COBRA_DISCOVERY_OK;
}

