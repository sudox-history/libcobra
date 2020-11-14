#include <stdio.h>

#define COBRA_TRANSPORT_SOCKET_PRIVATE
#include "transport_socket.h"

int main() {
    printf("%d\n", CORBA_PACKET_MAX_SIZE);
}