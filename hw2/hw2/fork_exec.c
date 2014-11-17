#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "constant.h"
#include "pipe.h"
#include "fork_exec.h"

char pipe_buff[SIZE_PIPE_BUFF];

/*
 * Handler (fork_and_exec)
 */
// simple exec (no pipe included), for last command (no following pipe)
int fork_and_exec_last(int connfd, char **cmd) {

    pid_t pid;
    pid = fork();

    if(pid < 0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid == 0) {           // if child

        // bind pipe in
        int fd_in = pipe_get();
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if(DEBUG)   debug_fork_and_exec_last(cmd, fd_in);
        
        // bind out to stdout
        dup2(connfd, STDOUT_FILENO);    // duplicate socket on stdout
        if(!DEBUG)  dup2(connfd, STDERR_FILENO);


        if( cmd[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);

    } else {                        // if parent

        int return_val;    
        waitpid(pid, &return_val, 0);
        if(WEXITSTATUS(return_val) == EXIT_FAILURE)  return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;

}

// simple exec (no pipe included, but for piping OUTs)
int fork_and_exec_pipe(int connfd, char **cmd, int p_n) {

    if(strcmp(cmd[0], "cat")==0 && cmd[1]==NULL)    return SKIP_SHIFT;

    // create pipe
    int *fd = pipe_create(p_n);

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        // output old_pipe content if exist
        int *old_pipe = get_old_pipe();
        if(old_pipe!=NULL) {

            memset(pipe_buff, 0, sizeof(pipe_buff)); 

            int count;
            if( (count = read(old_pipe[READ], pipe_buff, SIZE_PIPE_BUFF)) < 0 ) {
                fprintf(stderr, "read pipe connent error: %s\n", strerror(errno));
            }

            if(close(old_pipe[READ]) < 0) {
                fprintf(stderr, "pipe close error (old_pipe in): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            if(DEBUG)   debug_print_pipe_cat_content(count);

            if( write(fd[WRITE], pipe_buff, count) < 0 ) {
                fprintf(stderr, "write pipe connent error: %s\n", strerror(errno));
            }

        }

        // redirect STDOUT to pipe
        if(close(fd[READ]) < 0) {
            fprintf(stderr, "pipe close error (fd in): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        dup2(fd[WRITE], STDOUT_FILENO);
        if(!DEBUG)  dup2(connfd, STDERR_FILENO);

        // DEBUG
        if(DEBUG)   debug_print_pipe_map();

        // redirect STDIN to pipe_map[0][READ]
        int fd_in = pipe_get();
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if(cmd[0][0]=='/' || execvp(cmd[0], cmd)<0) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            close(fd[WRITE]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {                        // if parent

        int return_val;    
        waitpid(pid, &return_val, 0);
        if( close(fd[WRITE]) < 0 ) {
            fprintf(stderr, "pipe close error (fd out): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(WEXITSTATUS(return_val) == EXIT_FAILURE) return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;

}

// output to file ('>')
int fork_and_exec_file(int connfd, char **cmd, char *filepath) {

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        // bind stdout to file
        int fd_file = open(filepath, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(fd_file, STDOUT_FILENO);
        if(!DEBUG)  dup2(connfd, STDERR_FILENO);

        // redirect STDIN to pipe_map[0][READ]
        int fd_in = pipe_get();
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if( cmd[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            close(fd_file);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {                        // if parent

        int return_val;    
        waitpid(pid, &return_val, 0);
        if( WEXITSTATUS(return_val) == EXIT_FAILURE )  return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;

}

/*
 * Debug Functions
 */
void debug_print_pipe_cat_content(int count) {
    int k;
    fprintf(stderr, "---\n");
    for( k=0 ; k<count ; k++ ) {
        fprintf(stderr, "%c", pipe_buff[k]);
    }
    fprintf(stderr, "---\n");
}

