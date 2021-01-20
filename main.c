#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cobra.h"


void address(char *host) {
    printf("%s\n", host);
}

int main() {
    cobra_discovery_get_addresses(address);
}