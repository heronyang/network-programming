#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "constant.h"
#include "pipe.h"

int *pipe_map[MAX_PIPE_NUM];
int *old_pipe = NULL;

/*
 * Pipe Map
 */
int *pipe_create(int p_n) {

    int *fd = malloc(sizeof(int) * 2);
    if(pipe(fd) < 0)    fprintf(stderr, "pipe failed\n");

    if(pipe_map[p_n])   old_pipe = pipe_map[p_n];
    else                old_pipe = NULL;

    pipe_map[p_n] = fd;
    return fd;

}

int pipe_get() {

    if(!pipe_map[0])    return 0;
    return pipe_map[0][READ];

}

void pipe_shift() {

    int i;
    for(i=0 ; i<(MAX_PIPE_NUM-1) ; i++) pipe_map[i] = pipe_map[i+1];

}

void pipe_reset() {

    int i;
    for(i=0 ; i<MAX_PIPE_NUM ; i++) pipe_map[i] = NULL;

}

int *get_old_pipe() {
    return old_pipe;
}

/*
 * Debug
 */
void debug_fork_and_exec_last(char **argv, int fd_in) {
    int i;
    fprintf(stderr, "\n==========\n(rest)exec: ");
    for( i=0 ; argv[i]!=NULL ; i++ ){
        fprintf(stderr, ".%s", argv[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "last...\n");
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[i]) fprintf(stderr, "pipe_map[%d] = %p, [%d][%d]\n", i, pipe_map[i], pipe_map[i][READ], pipe_map[i][WRITE]);
    fprintf(stderr, "----\n");

    fprintf(stderr, "READ: %d\n", fd_in);
    fprintf(stderr, "WRITE: -\n");
}

void debug_print_pipe_map() {
    int i;
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[i]) fprintf(stderr, "pipe_map[%d] = %p, [%d][%d]\n", i, pipe_map[i], pipe_map[i][READ], pipe_map[i][WRITE]);
    fprintf(stderr, "----\n");
}

