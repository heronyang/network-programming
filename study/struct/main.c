#include <stdio.h>
#include <stdlib.h>

#define SIZE 10

typedef struct {
    int pid;
    char *name;
    char *ip;
    int port;
} Client;

int main() {

    Client **c_arr = NULL;
    c_arr = (Client **)malloc(sizeof(Client *) * SIZE);

    int i;
    for( i=0 ; i<SIZE ; i++) {
        c_arr[i] = malloc(sizeof(Client));
    }

    c_arr[4]->pid = 100;

    printf("pid = %d\n", c_arr[4]->pid);

    return 0;
}
