#include <stdlib.h>
#include <signal.h>
#include <stdio.h>  /* needed for printf() */

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
        fprintf(stderr, "unfinished client, can't close\n");
        return;
    }

    // release shared memory from system
    //fifo_finalize();
    //fifo_lock_close();
    fprintf(stderr, "exit program\n");
    exit(0);    // end program

}

