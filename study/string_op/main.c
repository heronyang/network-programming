#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 100

#define TRUE    1
#define FALSE   0

char *get_following(char *buff) {
    int i, start = FALSE;
    char *res = malloc(sizeof(char) * SIZE);
    for( i=0 ; i<strlen(buff) ; i++ ) {
        if( buff[i]!=' ' )  start = TRUE;
        if( start && buff[i]==' ' ) break;
    }
    memcpy(res, &buff[i+1], strlen(buff) - i);
    return res;
}

int main() {

    char ss[SIZE];
    char *s = " yell hey how are you";
    char *result;

    strcpy(ss, s);

    result = get_following(ss);

    printf("result = %s.\n", result);

    return 0;
}
