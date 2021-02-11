#define COBRA_DISCOVERY_PRIVATE
#include "cobra/discovery.h"

cobra_discovery_t *cobra_discovery_create() {
    cobra_discovery_t *discovery = malloc(sizeof(cobra_discovery_t));

    uv_loop_init(&discovery->loop);
    uv_mutex_init(&discovery->mutex_handle);
    cobra_async_init(&discovery->close_async, 1);

    uv_handle_set_data((uv_handle_t *)&discovery->udp_handle, discovery);
    uv_handle_set_data((uv_handle_t *)&discovery->timer_handle, discovery);
    cobra_async_set_data(&discovery->close_async, discovery);

    discovery->state = COBRA_DISCOVERY_STATE_CLOSED;
    discovery->closed_handlers_count = 0;

    cobra_async_set_callbacks(&discovery->close_async,
                              cobra__discovery_close_async_send_callback, NULL,
                              cobra__discovery_async_close_callback);

    cobra_buffer_init(&discovery->read_buffer, COBRA_DISCOVERY_PACKET_SIZE);

    discovery->found_callback = NULL;
    discovery->close_callback = NULL;

    return discovery;
}

cobra_discovery_err_t cobra_discovery_destroy(cobra_discovery_t *discovery) {
    uv_mutex_lock(&discovery->mutex_handle);

    if (discovery->state != COBRA_DISCOVERY_STATE_CLOSED) {
        uv_mutex_unlock(&discovery->mutex_handle);
        return COBRA_DISCOVERY_ERR_NOT_CLOSED;
    }

    uv_mutex_unlock(&discovery->mutex_handle);
    cobra_async_deinit(&discovery->close_async);
    cobra_buffer_deinit(&discovery->read_buffer);
    free(discovery);

    return COBRA_DISCOVERY_OK;
}

cobra_discovery_err_t cobra_discovery_get_addresses(cobra_discovery_t *discovery,
    cobra_discovery_addresses_cb addresses_callback) {
    uv_interface_address_t *addresses;
    int addresses_length;

    int result = uv_interface_addresses(&addresses, &addresses_length);
    if (result != 0)
        return COBRA_DISCOVERY_ERR_GETTING_ADDRESSES;

    for (int i = 0; i < addresses_length; i++) {
        char host[COBRA_DISCOVERY_HOST_STRLEN];
        uv_ip4_name(&addresses[i].address.address4, host,
                    COBRA_DISCOVERY_HOST_STRLEN);

        addresses_callback(discovery, host);
    }

    uv_free_interface_addresses(addresses, addresses_length);
    return COBRA_DISCOVERY_OK;
}