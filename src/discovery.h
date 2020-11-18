#ifndef COBRA_DISCOVERY_H
#define COBRA_DISCOVERY_H

#ifdef COBRA_DISCOVERY_PRIVATE
#include <uv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "buffer.h"
#endif

#define COBRA_DISCOVERY_OK 0
#define COBRA_DISCOVERY_ERR_ALREADY_STARTED 1
#define COBRA_DISCOVERY_ERR_BINDING 2
#define COBRA_DISCOVERY_ERR_STARTING_BROADCAST 3
#define COBRA_DISCOVERY_ERR_SENDING 4

#ifdef COBRA_DISCOVERY_PRIVATE
uint8_t COBRA_DISCOVERY_PACKET[] = {8, 100, 193, 210, 19};
#define COBRA_DISCOVERY_ANY_ADDR "0.0.0.0"
#define COBRA_DISCOVERY_BROADCAST_ADDR "239.255.255.250"
#define COBRA_DISCOVERY_PORT 55669
#define COBRA_DISCOVERY_TIMEOUT 5000
#define COBRA_DISCOVERY_HOST_STRLEN 16
#endif

typedef struct cobra_discovery_t cobra_discovery_t;

typedef void (*cobra_discovery_found_cb)
        (cobra_discovery_t *discovery, char *host);

typedef void (*cobra_discovery_close_cb)
        (cobra_discovery_t *discovery, int error);

#ifdef COBRA_DISCOVERY_PRIVATE
struct cobra_discovery_t {
    /* Libuv handles */
    uv_loop_t loop;
    uv_udp_t udp_handle;
    uv_timer_t timer_handle;

    /* Management */
    bool is_listening;
    bool is_scanning;
    bool is_closing;
    int close_error;

    /* Read */
    cobra_buffer_t read_buffer;

    /* Callbacks */
    cobra_discovery_found_cb on_found;
    cobra_discovery_close_cb on_close;

    /* Other */
    void *data;
};
#endif

cobra_discovery_t *cobra_discovery_create();
void cobra_discovery_destroy(cobra_discovery_t *discovery);

int cobra_discovery_listen(cobra_discovery_t *discovery);
int cobra_discovery_scan(cobra_discovery_t *discovery);

int cobra_discovery_close(cobra_discovery_t *discovery);

#endif //COBRA_DISCOVERY_H
