#include <stdio.h>
#include <stdlib.h>
#include "client_name.h"

int main() {

    pid_t pid;
    pid = fork();

    fifo_setup();
    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child
        fifo_w();
        fifo_r();
        fifo_close();
    } else {
        fifo_close();
    }




    return 0;
}
