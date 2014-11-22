#ifndef PIPE_H
#define PIPE_H

int *pipe_create(int p_n);
int pipe_get();
void pipe_shift();
void pipe_reset();

int *get_old_pipe();

void debug_fork_and_exec_last(char **argv, int fd_in);
void debug_print_pipe_map();

#endif
