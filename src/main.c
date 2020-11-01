#include <stdio.h>
#include "transport_connection.h"

int main() {
    transport_connection_t *connection = transport_connection_create();
    transport_connection_connect(connection, "localhost", "6001");
}
