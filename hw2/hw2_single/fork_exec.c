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
#include "variable.h"
#include "clients.h"
//#include "broadcast.h"

char pipe_buff[SIZE_PIPE_BUFF];
int fifo_fd[CLIENT_MAX_NUM][CLIENT_MAX_NUM];

/*
 * Handler (fork_and_exec)
 */
// simple exec (no pipe included), for last command (no following pipe)
int fork_and_exec_last(int connfd, char **cmd) {

    int client_id = clients_get_id_from_socket(connfd);

    pid_t pid;
    pid = fork();

    if(pid < 0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid == 0) {           // if child

        // bind pipe in
        int fd_in = pipe_get(client_id);
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if(DEBUG)   debug_fork_and_exec_last(client_id, cmd, fd_in);
        
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

    // if(strcmp(cmd[0], "cat")==0 && cmd[1]==NULL)    return SKIP_SHIFT;

    // create pipe
    int client_id = clients_get_id_from_socket(connfd);
    int *fd = pipe_create(client_id, p_n);

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        // output old_pipe content if exist
        int *op = get_old_pipe(client_id);
        if(op!=NULL) {

            memset(pipe_buff, 0, sizeof(pipe_buff)); 

            int count;
            if( (count = read(op[READ], pipe_buff, SIZE_PIPE_BUFF)) < 0 ) {
                fprintf(stderr, "read pipe connent error: %s\n", strerror(errno));
            }

            if(close(op[READ]) < 0) {
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
        if(DEBUG)   debug_print_pipe_map(client_id);

        // redirect STDIN to pipe_map[0][READ]
        int fd_in = pipe_get(client_id);
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

    int client_id = clients_get_id_from_socket(connfd);

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
        int fd_in = pipe_get(client_id);
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

int fork_and_exec_fifo_in(int connfd, char **cmd, int source_id) {

    int client_id = clients_get_id_from_socket(connfd);

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        dup2(connfd, STDOUT_FILENO);
        if(!DEBUG)  dup2(connfd, STDERR_FILENO);

        // redirect STDIN to pipe_map[0][READ]
        if(DEBUG)   fprintf(stderr, "fifo_in: fifo_path: %sclient_%d_%d\n", FIFO_PATH_DIR, source_id, client_id);
        dup2(fifo_fd[source_id][client_id], STDIN_FILENO);

        if( cmd[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {

        int return_val;    
        waitpid(pid, &return_val, 0);
        if( WEXITSTATUS(return_val) == EXIT_FAILURE )  return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;

}

int fork_and_exec_fifo_out(int connfd, char **cmd, int target_id) {

    int client_id = clients_get_id_from_socket(connfd);

    int fd;
    char fifo_path[PATH_LENGTH];

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        sprintf(fifo_path, "%sclient_%d_%d", FIFO_PATH_DIR, client_id, target_id);
        fprintf(stderr, "fifo_path:%s\n", fifo_path);
        if((fd = open(fifo_path, O_NONBLOCK | O_WRONLY) ) < 0) {
            perror("open");
        }

        dup2(fd, STDOUT_FILENO);
        if(!DEBUG)  dup2(fd, STDERR_FILENO);

        // redirect STDIN to pipe_map[0][READ]
        int fd_in = pipe_get(client_id);
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if( cmd[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            close(fd);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {

        int return_val;    
        waitpid(pid, &return_val, 0);
        if( WEXITSTATUS(return_val) == EXIT_FAILURE )  return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;
}

int fork_and_exec_fifo_in_out(int connfd, char **cmd, int source_id, int target_id) {

    int client_id = clients_get_id_from_socket(connfd);

    int fd;
    char fifo_path[PATH_LENGTH];

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid ==0) {           // if child

        // out
        sprintf(fifo_path, "%sclient_%d_%d", FIFO_PATH_DIR, client_id, target_id);
        fprintf(stderr, "fifo_path:%s\n", fifo_path);
        if((fd = open(fifo_path, O_NONBLOCK | O_WRONLY) ) < 0) {
            perror("open");
        }

        dup2(fd, STDOUT_FILENO);
        if(!DEBUG)  dup2(fd, STDERR_FILENO);

        // in
        if(DEBUG)   fprintf(stderr, "fifo_in: fifo_path: %sclient_%d_%d\n", FIFO_PATH_DIR, source_id, client_id);
        dup2(fifo_fd[source_id][client_id], STDIN_FILENO);

        if( cmd[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            fprintf(stderr, "Unknown command: [%s].\n", cmd[0]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {

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

