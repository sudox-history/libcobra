#define COBRA_DISCOVERY_PRIVATE
#include "cobra/discovery.h"

cobra_discovery_err_t cobra_discovery_listen(cobra_discovery_t *discovery) {
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
    cobra_async_bind(&discovery->close_async, &discovery->loop);
    uv_mutex_unlock(&discovery->mutex_handle);

    cobra__discovery_listen_ctx_t *listen_ctx =
        malloc(sizeof(cobra__discovery_listen_ctx_t));
    listen_ctx->discovery = discovery;

    uv_thread_create(&discovery->thread_handle, cobra__discovery_listen_thread,
                     listen_ctx);

    return COBRA_DISCOVERY_OK;
}

void cobra__discovery_listen_thread(void *data) {
    cobra__discovery_listen_ctx_t *listen_ctx = data;
    cobra_discovery_t *discovery = listen_ctx->discovery;

    // Don't forget to free context
    free(discovery);

    uv_mutex_lock(&discovery->mutex_handle);

    // During thread creation user could run cobra_discovery_close()
    if (discovery->state != COBRA_DISCOVERY_STATE_CLOSING) {
        discovery->state = COBRA_DISCOVERY_STATE_LISTENING;
        uv_mutex_unlock(&discovery->mutex_handle);

        struct sockaddr_in bind_addr;
        uv_ip4_addr(COBRA_DISCOVERY_ANY_ADDR, COBRA_DISCOVERY_PORT, &bind_addr);

        if (uv_udp_bind(&discovery->udp_handle,
                        (const struct sockaddr *)&bind_addr, 0))
            cobra__discovery_close(discovery, COBRA_DISCOVERY_ERR_BINDING);

        if ((uv_udp_set_membership(&discovery->udp_handle,
                                   COBRA_DISCOVERY_MULTICAST_ADDR, NULL,
                                   UV_JOIN_GROUP)))
            cobra__discovery_close(discovery,
                                   COBRA_DISCOVERY_ERR_JOINING_GROUP);

        uv_udp_recv_start(&discovery->udp_handle,
                          cobra__discovery_listener_alloc_callback,
                          cobra__discovery_listener_read_callback);
    } else {
        uv_mutex_unlock(&discovery->mutex_handle);
    }

    uv_run(&discovery->loop, UV_RUN_DEFAULT);
}

void cobra__discovery_listener_alloc_callback(uv_handle_t *udp_handle,
                                              size_t length,
                                              uv_buf_t *buffer) {
    cobra_discovery_t *discovery = udp_handle->data;

    buffer->base = (char *)cobra_buffer_write_pointer(&discovery->read_buffer);
    buffer->len = cobra_buffer_capacity(&discovery->read_buffer);
}

void cobra__discovery_listener_read_callback(uv_udp_t *udp_handle,
                                             ssize_t length,
                                             const uv_buf_t *buffer,
                                             const struct sockaddr *addr,
                                             unsigned _) {
    cobra_discovery_t *discovery = udp_handle->data;

    if (length <= 0)
        return;

    // Skipping wrote bytes
    cobra_buffer_write_void(&discovery->read_buffer, length);

    if (!cobra_buffer_equals(&discovery->read_buffer, COBRA_DISCOVERY_PACKET,
                             sizeof(COBRA_DISCOVERY_PACKET)))
        return;

    uv_buf_t send_buffer = {.base = (char *)COBRA_DISCOVERY_PACKET,
                            .len = sizeof(COBRA_DISCOVERY_PACKET)};

    uv_udp_send_t *send_request = malloc(sizeof(uv_udp_send_t));
    uv_req_set_data(send_request, discovery);

    uv_udp_send(send_request, &discovery->udp_handle, &send_buffer, 1, addr,
                cobra__discovery_listener_send_callback);

    cobra_buffer_clear(&discovery->read_buffer);
}

void cobra__discovery_listener_send_callback(uv_udp_send_t *send_request,
                                             int error) {
    cobra_discovery_t *discovery = send_request->data;
    free(send_request);

    if (error) {
        cobra__discovery_close(discovery, COBRA_DISCOVERY_ERR_SENDING_FRAME);
        return;
    }
}