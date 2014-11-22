#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "constant.h"
#include "pipe.h"

int *pipe_map[CLIENT_MAX_NUM][MAX_PIPE_NUM];
int *old_pipe[CLIENT_MAX_NUM];

/*
 * Pipe Map
 */
int *pipe_create(int client_id, int p_n) {

    int *fd = malloc(sizeof(int) * 2);
    if(pipe(fd) < 0)    fprintf(stderr, "pipe failed\n");

    if(pipe_map[client_id][p_n])    old_pipe[client_id] = pipe_map[client_id][p_n];
    else                            old_pipe[client_id] = NULL;

    pipe_map[client_id][p_n] = fd;
    return fd;

}

int pipe_get(int client_id) {

    if(!pipe_map[client_id][0]) return 0;
    return pipe_map[client_id][0][READ];

}

void pipe_shift(int client_id) {

    int i;
    for(i=0 ; i<(MAX_PIPE_NUM-1) ; i++) pipe_map[client_id][i] = pipe_map[client_id][i+1];

}

void pipe_reset(int client_id) {

    int i;
    for(i=0 ; i<MAX_PIPE_NUM ; i++) pipe_map[client_id][i] = NULL;

}

int *get_old_pipe(int client_id) {
    return old_pipe[client_id];
}

/*
 * Debug
 */
void debug_fork_and_exec_last(int client_id, char **argv, int fd_in) {
    int i;
    fprintf(stderr, "\n==========\n(rest)exec: ");
    for( i=0 ; argv[i]!=NULL ; i++ ){
        fprintf(stderr, ".%s", argv[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "last...\n");
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[client_id][i]) fprintf(stderr, "pipe_map[%d][%d] = %p, [%d][%d]\n", client_id, i, pipe_map[client_id][i], pipe_map[client_id][i][READ], pipe_map[client_id][i][WRITE]);
    fprintf(stderr, "----\n");

    fprintf(stderr, "READ: %d\n", fd_in);
    fprintf(stderr, "WRITE: -\n");
}

void debug_print_pipe_map(int client_id) {
    int i;
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[client_id][i]) fprintf(stderr, "pipe_map[%d][%d] = %p, [%d][%d]\n", client_id, i, pipe_map[client_id][i], pipe_map[client_id][i][READ], pipe_map[client_id][i][WRITE]);
    fprintf(stderr, "----\n");
}

