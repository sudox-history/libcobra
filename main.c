#include <stdio.h>
#include <stdlib.h>
#include "cobra.h"

int *main_ptr = NULL;
int *func_ptr = NULL;
void func() {
    int a;
    func_ptr = &a;
}
int main()
{
    int a;
    main_ptr = &a;
    func();
    (main_ptr > func_ptr) ? printf("DOWN\n") : printf("UP\n");

    char *ab = malloc(20);
    printf("%lu\n", main_ptr);
    printf("%lu\n", func_ptr);
    printf("%lu\n", ab);
    return 0;
}