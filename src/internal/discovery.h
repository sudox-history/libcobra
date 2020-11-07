#ifndef COBRA_DISCOVERY_H
#define COBRA_DISCOVERY_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <uv.h>

#include "buffer.h"

typedef struct cobra_discovery_t cobra_discovery_t;

typedef void (*found_cb)(cobra_discovery_t *discovery, char *host);

#ifdef COBRA_DISCOVERY_PRIVATE
uint8_t COBRA_DISCOVERY_PACKET[5] = {8, 100, 193, 210, 19};

#define COBRA_DISCOVERY_ADDR_ANY "0.0.0.0"
#define COBRA_DISCOVERY_ADDR "239.255.255.250"
#define COBRA_DISCOVERY_PORT 55669

#define COBRA_DISCOVERY_OK 0
#define COBRA_DISCOVERY_ERR_BINDING 1
#define COBRA_DISCOVERY_ERR_LISTENING 2

#define COBRA_DISCOVERY_DISCOVER_TIMEOUT 5000
#endif

#ifdef COBRA_DISCOVERY_PRIVATE
struct cobra_discovery_t {
    /* Main loop */
    uv_loop_t loop;

    /* UDP Handle */
    uv_udp_t udp_handle;
    uv_timer_t timer_handle;

    /* Requests */
    uv_udp_send_t send_req;

    /* Management */
    cobra_buffer_t *buffer;
    bool scanning;
    bool listening;

    /* Callbacks */
    found_cb on_found;
};
#endif

cobra_discovery_t *cobra_discovery_create(found_cb on_found);
void cobra_discovery_destroy(cobra_discovery_t *discovery);

int cobra_discovery_listen(cobra_discovery_t *discovery);
int cobra_discovery_scan(cobra_discovery_t *discovery);

void cobra_discovery_listen_close(cobra_discovery_t *discovery);
void cobra_discovery_scan_close(cobra_discovery_t *discovery);

#endif //COBRA_DISCOVERY_H
