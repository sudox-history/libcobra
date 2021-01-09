#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_set_callbacks(cobra_async_t *async, cobra_async_main_callback main_callback) {
    async->main_callback = main_callback;
}
