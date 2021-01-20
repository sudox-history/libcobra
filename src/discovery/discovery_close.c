#define COBRA_DISCOVERY_PRIVATE
#include "cobra/discovery.h"

cobra_discovery_err_t cobra_discovery_close(cobra_discovery_t *sock) {
    return cobra__discovery_close(sock, COBRA_DISCOVERY_OK);
}

cobra_discovery_err_t cobra__discovery_close(cobra_discovery_t *discovery,
                                             cobra_discovery_err_t error) {
    uv_mutex_lock(&discovery->mutex_handle);

    if (discovery->state == COBRA_DISCOVERY_STATE_CLOSED) {
        uv_mutex_unlock(&discovery->mutex_handle);
        return COBRA_DISCOVERY_ERR_ALREADY_CLOSED;
    }

    // If the discovery is already closing - do nothing
    if (discovery->state == COBRA_DISCOVERY_STATE_CLOSING) {
        uv_mutex_unlock(&discovery->mutex_handle);
        return COBRA_DISCOVERY_OK;
    }

    discovery->state = COBRA_DISCOVERY_STATE_CLOSING;
    uv_mutex_unlock(&discovery->mutex_handle);

    cobra__discovery_close_ctx_t *close_ctx =
        malloc(sizeof(cobra__discovery_close_ctx_t));

    close_ctx->discovery = discovery;
    close_ctx->error = error;

    // That function is thread safe
    cobra_async_send(&discovery->close_async, close_ctx);

    return COBRA_DISCOVERY_OK;
}

void cobra__discovery_close_async_send_callback(cobra_async_t *async,
                                                void *data) {
    cobra__discovery_close_ctx_t *close_ctx = data;

    cobra_discovery_t *discovery = close_ctx->discovery;
    cobra_discovery_err_t error = close_ctx->error;

    // Don't forget to free context
    free(close_ctx);

    // Close handlers and async structures.
    // It's safe to close them here because only one callback can be executed at
    // time
    uv_close((uv_handle_t *)&discovery->udp_handle,
             cobra__discovery_close_callback);
    uv_close((uv_handle_t *)&discovery->timer_handle,
             cobra__discovery_close_callback);
    cobra_async_close(&discovery->close_async);

    discovery->close_error = error;
    discovery->closed_handlers_count = 0;
}

static void cobra__discovery_common_close_callback(
    cobra_discovery_t *discovery) {
    discovery->closed_handlers_count++;

    if (discovery->closed_handlers_count ==
        COBRA_DISCOVERY_TOTAL_HANDLERS_COUNT) {
        uv_mutex_lock(&discovery->mutex_handle);
        cobra_discovery_close_cb close_callback = discovery->close_callback;
        uv_mutex_unlock(&discovery->mutex_handle);

        if (close_callback != NULL)
            close_callback(discovery, discovery->close_error);
    }
}

void cobra__discovery_close_callback(uv_handle_t *handle) {
    cobra__discovery_common_close_callback(uv_handle_get_data(handle));
}

void cobra__discovery_async_close_callback(cobra_async_t *async) {
    cobra__discovery_common_close_callback(cobra_async_get_data(async));
}