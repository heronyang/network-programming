#ifndef PIPE_H
#define PIPE_H

int *pipe_create(int client_id, int p_n);
int pipe_get(int client_id);
void pipe_shift(int client_id);
void pipe_reset(int client_id);

int *get_old_pipe(int client_id);

void debug_fork_and_exec_last(int client_id, char **argv, int fd_in);
void debug_print_pipe_map(int client_id);

#endif
