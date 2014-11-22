#include <stdio.h>
#include <string.h>

typedef struct {
    char a;
    char str[200];
} S;

S gs[100];

int main() {

    printf("size: %d\n", sizeof(S));

    strcpy(gs[50].str, "hello");
    printf(">> %s\n", gs[50].str);


    return 0;
}
