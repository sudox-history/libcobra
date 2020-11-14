#include <stdio.h>
#include "tools.h"

int main() {
    uint8_t data[] = {1, 2, 3, 4};
    cobra_tools_array_reverse(data, 4);
    cobra_tools_array_print(data, 4);
}