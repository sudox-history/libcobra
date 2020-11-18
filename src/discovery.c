#define COBRA_DISCOVERY_PRIVATE
#include "discovery.h"

cobra_discovery_t *cobra_discovery_create() {
    cobra_discovery_t *discovery = malloc(sizeof(cobra_discovery_t));

    uv_loop_init(&discovery->loop);
    uv_timer_init(&discovery->loop, &discovery->timer_handle);
    uv_udp_init(&discovery->loop, &discovery->udp_handle);

    cobra_buffer_init(&discovery->listener_read_buffer, sizeof(COBRA_DISCOVERY_PACKET));
    discovery->is_listening = false;
    discovery->is_scanning = false;
    discovery->is_closing = false;

    discovery->on_found = NULL;
    discovery->on_close = NULL;

    return discovery;
}

void cobra_discovery_destroy(cobra_discovery_t *discovery) {
    cobra_buffer_deinit(&discovery->listener_read_buffer);
    free(discovery);
}

void cobra__discovery_on_close(uv_handle_t *handle) {
    cobra_discovery_t *discovery = handle->data;
}

void cobra__discovery_close(cobra_discovery_t *discovery, int error) {
    if (discovery->is_closing)
        return;

    discovery->is_closing = true;
    discovery->close_error = error;

    if (discovery->is_scanning)
        uv_timer_stop(&discovery->timer_handle);

    uv_close((uv_handle_t *) &discovery->udp_handle,
             cobra__discovery_on_close);
}

void cobra__discovery_on_listener_alloc(uv_handle_t *handle, size_t _, uv_buf_t *read_buffer) {
    cobra_discovery_t *discovery = handle->data;

    read_buffer->base = (char *) cobra_buffer_write_pointer(&discovery->listener_read_buffer);
    read_buffer->len = cobra_buffer_capacity(&discovery->listener_read_buffer);
}

void cobra__discovery_on_listener_send(uv_udp_send_t *req, int error) {
    cobra_discovery_t *discovery = req->data;

    if (error) {
        cobra__discovery_close(discovery, COBRA_DISCOVERY_ERR_SENDING);
        return;
    }

    free(req);
}

void cobra__discovery_on_listener_data(uv_udp_t *handle,
                                       ssize_t read_length,
                                       const uv_buf_t *_,
                                       const struct sockaddr *addr,
                                       unsigned _2) {
    cobra_discovery_t *discovery = handle->data;

    if (read_length <= 0)
        return;

    // Skipping wrote bytes
    cobra_buffer_write_void(&discovery->listener_read_buffer, read_length);

    if (cobra_buffer_equals(&discovery->listener_read_buffer,
                            COBRA_DISCOVERY_PACKET,
                            sizeof(COBRA_DISCOVERY_PACKET)))
        return;

    uv_buf_t write_buffer = {
            .base = (char *) COBRA_DISCOVERY_PACKET,
            .len = sizeof(COBRA_DISCOVERY_PACKET)
    };

    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
    send_req->data = discovery;

    uv_udp_send(send_req,
                &discovery->udp_handle,
                &write_buffer,
                1,
                addr,
                cobra__discovery_on_listener_send);
}

int cobra_discovery_listen(cobra_discovery_t *discovery) {
    if (discovery->is_listening || discovery->is_scanning)
        return COBRA_DISCOVERY_ERR_ALREADY_STARTED;

    struct sockaddr_in bind_addr;
    uv_ip4_addr(COBRA_DISCOVERY_ANY_ADDR, COBRA_DISCOVERY_PORT, &bind_addr);

    if (uv_udp_bind(&discovery->udp_handle,
                    (const struct sockaddr *) &bind_addr,
                    0))
        return COBRA_DISCOVERY_ERR_BINDING;

    discovery->is_listening = true;

    uv_udp_recv_start(&discovery->udp_handle,
                      cobra__discovery_on_listener_alloc,
                      cobra__discovery_on_listener_data);

    return COBRA_DISCOVERY_OK;
}