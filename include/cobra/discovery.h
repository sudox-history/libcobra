#ifndef COBRA_DISCOVERY_H
#define COBRA_DISCOVERY_H

#ifdef COBRA_DISCOVERY_PRIVATE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>
#include "cobra/buffer.h"
#endif

#define COBRA_DISCOVERY_OK 0
#define COBRA_DISCOVERY_ERR_ALREADY_STARTED 1
#define COBRA_DISCOVERY_ERR_BINDING 2
#define COBRA_DISCOVERY_ERR_JOINING_GROUP 3
#define COBRA_DISCOVERY_ERR_SENDING 4

#ifdef COBRA_DISCOVERY_PRIVATE
uint8_t COBRA_DISCOVERY_PACKET[] = {8, 100, 193, 210, 19};
#define COBRA_DISCOVERY_ANY_ADDR "0.0.0.0"
#define COBRA_DISCOVERY_MULTICAST_ADDR "239.255.255.250"
#define COBRA_DISCOVERY_PORT 55669
#define COBRA_DISCOVERY_TIMEOUT 5000
#define COBRA_DISCOVERY_HOST_STRLEN 16
#endif

typedef struct cobra_discovery_t cobra_discovery_t;

typedef void (*cobra_discovery_found_cb)(cobra_discovery_t *discovery,
                                         char *host);

typedef void (*cobra_discovery_close_cb)(cobra_discovery_t *discovery,
                                         int error);

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

void cobra_discovery_set_data(cobra_discovery_t *discovery, void *data);
void *cobra_discovery_get_data(cobra_discovery_t *discovery);

void cobra_discovery_set_callbacks(cobra_discovery_t *discovery,
                                   cobra_discovery_found_cb on_found,
                                   cobra_discovery_close_cb on_close);

#endif  // COBRA_DISCOVERY_H
