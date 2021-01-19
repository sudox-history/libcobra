#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cobra.h"

void on_close(cobra_discovery_t *discovery, cobra_discovery_err_t error) {
    printf("Close callback invoked, reason %d\n", error);
    cobra_discovery_destroy(discovery);
}

void on_found(cobra_discovery_t *discovery, char *host) {
    printf("Found callback invoked: %s\n", host);
}

int main() {
    cobra_discovery_t *discovery = cobra_discovery_create();
    cobra_discovery_set_callbacks(discovery, on_found, on_close);

    cobra_discovery_listen(discovery);

    scanf("%d");
}