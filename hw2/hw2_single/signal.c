#include <stdlib.h>
#include <signal.h>
#include <stdio.h>  /* needed for printf() */
#include <sys/wait.h>

#include "signal.h"
#include "variable.h"

int client_count;

/* 
 * Signal
 */
void signal_init() {

    /* SIGNAL: catch control c */
    if (signal(SIGINT, catch_int) == SIG_ERR) {
        fprintf(stderr, "can't catch SIGINT\n");
    }

}

void catch_chld(int snum) {

    int pid;
    int status;

    pid = wait(&status);
    fprintf(stderr, "parent: child process pid=%d exited with value %d\n", pid, WEXITSTATUS(status));

}

void catch_int(int i) {

    if(client_count!=0) {
        fprintf(stderr, "\nclient_count = %d, unfinished client, please kill zoombie\n", client_count);
    }

    // release shared memory from system
    //fifo_finalize();
    fprintf(stderr, "exit program\n");
    exit(0);    // end program

}

