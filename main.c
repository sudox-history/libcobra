#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cobra.h"

void on_found(cobra_discovery_t *discovery, char *host) {
    printf("FOUND ADDRESS: %s\n", host);
}

int main() {
    cobra_discovery_t *discovery = cobra_discovery_create();

    cobra_discovery_set_callbacks(discovery, on_found, NULL);
    cobra_discovery_scan(discovery);

    scanf("%d");
}