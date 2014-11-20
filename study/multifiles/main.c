#include <stdio.h>
#include "tool.h"

int a;

int main() {
    a = 4;
    printf("a = %d\n", a);

    test();

    printf("a = %d\n", a);

    a = 6;
    test();

    return 0;
}
