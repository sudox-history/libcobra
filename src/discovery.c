#define COBRA_DISCOVERY_PRIVATE
#include "discovery.h"

cobra_discovery_t *cobra_discovery_create() {
    cobra_discovery_t *discovery = malloc(sizeof(cobra_discovery_t));

    uv_loop_init(&discovery->loop);
    uv_udp_init(&discovery->loop, &discovery->udp_handle);
    uv_timer_init(&discovery->loop, &discovery->timer_handle);
    discovery->loop.data = discovery;
    discovery->udp_handle.data = discovery;
    discovery->timer_handle.data = discovery;

    cobra_buffer_init(&discovery->read_buffer, sizeof(COBRA_DISCOVERY_PACKET));
    discovery->is_listening = false;
    discovery->is_scanning = false;
    discovery->is_closing = false;

    discovery->on_found = NULL;
    discovery->on_close = NULL;

    return discovery;
}

void cobra_discovery_destroy(cobra_discovery_t *discovery) {
    cobra_buffer_deinit(&discovery->read_buffer);
    free(discovery);
}

void cobra__discovery_on_close(uv_handle_t *handle) {
    cobra_discovery_t *discovery = handle->data;

    // Necessary to re-init handles after uv_close
    uv_udp_init(&discovery->loop, &discovery->udp_handle);
    if (discovery->is_scanning)
        uv_timer_init(&discovery->loop, &discovery->timer_handle);

    discovery->is_listening = false;
    discovery->is_scanning = false;
    discovery->is_closing = false;

    cobra_buffer_clear(&discovery->read_buffer);
    if (discovery->on_close)
        discovery->on_close(discovery, discovery->close_error);
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

void cobra__discovery_on_send(uv_udp_send_t *send_req, int error) {
    cobra_discovery_t *discovery = send_req->data;

    if (error) {
        cobra__discovery_close(discovery, COBRA_DISCOVERY_ERR_SENDING);
        return;
    }

    free(send_req);
}

void cobra__discovery_on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *read_buffer) {
    cobra_discovery_t *discovery = handle->data;

    read_buffer->base = (char *) cobra_buffer_write_pointer(&discovery->read_buffer);
    read_buffer->len = cobra_buffer_capacity(&discovery->read_buffer);
}

void cobra__discovery_on_listener_data(uv_udp_t *handle,
                                       ssize_t read_length,
                                       const uv_buf_t *_,
                                       const struct sockaddr *addr,
                                       unsigned _1) {
    cobra_discovery_t *discovery = handle->data;

    if (read_length <= 0)
        return;

    // Skipping wrote bytes
    cobra_buffer_write_void(&discovery->read_buffer, read_length);

    if (!cobra_buffer_equals(&discovery->read_buffer,
                             COBRA_DISCOVERY_PACKET,
                             sizeof(COBRA_DISCOVERY_PACKET)))
        return;

    uv_buf_t send_buffer = {
            .base = (char *) COBRA_DISCOVERY_PACKET,
            .len = sizeof(COBRA_DISCOVERY_PACKET)
    };

    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
    send_req->data = discovery;

    uv_udp_send(send_req,
                &discovery->udp_handle,
                &send_buffer,
                1,
                addr,
                cobra__discovery_on_send);

    cobra_buffer_clear(&discovery->read_buffer);
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

    if ((uv_udp_set_membership(&discovery->udp_handle,
                               COBRA_DISCOVERY_MULTICAST_ADDR,
                               NULL,
                               UV_JOIN_GROUP)))
        return COBRA_DISCOVERY_ERR_JOINING_GROUP;

    discovery->is_listening = true;

    uv_udp_recv_start(&discovery->udp_handle,
                      cobra__discovery_on_alloc,
                      cobra__discovery_on_listener_data);

    uv_run(&discovery->loop, UV_RUN_DEFAULT);
    return COBRA_DISCOVERY_OK;
}

void cobra__discovery_on_scanner_time(uv_timer_t *handle) {
    cobra_discovery_t *discovery = handle->data;

    struct sockaddr_in addr;
    uv_ip4_addr(COBRA_DISCOVERY_MULTICAST_ADDR, COBRA_DISCOVERY_PORT, &addr);

    uv_buf_t send_buffer = {
            .base = (char *) COBRA_DISCOVERY_PACKET,
            .len = 5
    };

    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
    send_req->data = discovery;

    uv_udp_send(send_req,
                &discovery->udp_handle,
                &send_buffer,
                1,
                (const struct sockaddr *) &addr,
                cobra__discovery_on_send);
}

void cobra__discovery_on_scanner_data(uv_udp_t *handle,
                                      ssize_t read_length,
                                      const uv_buf_t *_,
                                      const struct sockaddr *addr,
                                      unsigned _1) {
    cobra_discovery_t *discovery = handle->data;

    if (read_length <= 0)
        return;

    // Skipping wrote bytes
    cobra_buffer_write_void(&discovery->read_buffer, read_length);

    if (!cobra_buffer_equals(&discovery->read_buffer,
                             COBRA_DISCOVERY_PACKET,
                             sizeof(COBRA_DISCOVERY_PACKET)))
        return;

    char host[COBRA_DISCOVERY_HOST_STRLEN];
    uv_ip4_name((const struct sockaddr_in *) addr, host, COBRA_DISCOVERY_HOST_STRLEN);

    if (discovery->on_found)
        discovery->on_found(discovery, host);

    cobra_buffer_clear(&discovery->read_buffer);
}

int cobra_discovery_scan(cobra_discovery_t *discovery) {
    if (discovery->is_listening || discovery->is_scanning)
        return COBRA_DISCOVERY_ERR_ALREADY_STARTED;

    struct sockaddr_in bind_addr;
    uv_ip4_addr(COBRA_DISCOVERY_ANY_ADDR, COBRA_DISCOVERY_PORT, &bind_addr);

    if (uv_udp_bind(&discovery->udp_handle,
                    (const struct sockaddr *) &bind_addr,
                    0))
        return COBRA_DISCOVERY_ERR_BINDING;

    discovery->is_scanning = true;

    uv_timer_start(&discovery->timer_handle,
                   cobra__discovery_on_scanner_time,
                   0,
                   COBRA_DISCOVERY_TIMEOUT);

    uv_udp_recv_start(&discovery->udp_handle,
                      cobra__discovery_on_alloc,
                      cobra__discovery_on_scanner_data);

    uv_run(&discovery->loop, UV_RUN_DEFAULT);
    return COBRA_DISCOVERY_OK;
}

int cobra_discovery_close(cobra_discovery_t *discovery) {
    if (!discovery->is_listening && !discovery->is_scanning)
        return COBRA_DISCOVERY_ERR_ALREADY_STARTED;

    cobra__discovery_close(discovery, COBRA_DISCOVERY_OK);
    return COBRA_DISCOVERY_OK;
}

void cobra_discovery_set_data(cobra_discovery_t *discovery, void *data) {
    discovery->data = data;
}

void *cobra_discovery_get_data(cobra_discovery_t *discovery) {
    return discovery->data;
}

void cobra_discovery_set_callbacks(cobra_discovery_t *discovery,
                                   cobra_discovery_found_cb on_found,
                                   cobra_discovery_close_cb on_close) {
    discovery->on_found = on_found;
    discovery->on_close = on_close;
}