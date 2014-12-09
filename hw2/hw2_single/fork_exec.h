#ifndef FORK_EXEC_H
#define FORK_EXEC_H

//
int fork_and_exec_last(int connfd, char **cmd);
int fork_and_exec_pipe(int connfd, char **cmd, int p_n);
int fork_and_exec_file(int connfd, char **cmd, char *filepath);

int fork_and_exec_fifo_in(int connfd, char **cmd, int source_id);
int fork_and_exec_fifo_out(int connfd, char **cmd, int target_id);
int fork_and_exec_fifo_in_out(int connfd, char **cmd, int source_id, int target_id);

//
void debug_print_pipe_cat_content(int count);

#endif
