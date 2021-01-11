#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_lock(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
}

void cobra_buffer_unlock(cobra_buffer_t *buffer) {
    uv_mutex_unlock(&buffer->mutex_handle);
}
