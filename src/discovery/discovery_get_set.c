#define COBRA_DISCOVERY_PRIVATE
#include "cobra/discovery.h"

void cobra_discovery_set_callbacks(cobra_discovery_t *discovery,
                                   cobra_discovery_found_cb found_callback,
                                   cobra_discovery_close_cb close_callback) {
    uv_mutex_lock(&discovery->mutex_handle);
    discovery->found_callback = found_callback;
    discovery->close_callback = close_callback;
    uv_mutex_unlock(&discovery->mutex_handle);
}

void cobra_discovery_set_data(cobra_discovery_t *discovery, void *data) {
    discovery->data = data;
}

void *cobra_discovery_get_data(cobra_discovery_t *discovery) {
    return discovery->data;
}