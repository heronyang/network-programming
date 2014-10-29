/*
 * Network Programming - HW1
 * Author:      Heron Yang
 * Director:    I-Chen Wu (Prof.)
 * Date:        Sat Oct 18 15:19:06 CST 2014
 *
 * Check NOTE.md for details.
 */

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

#define DEBUG           0

#define SIZE_SEND_BUFF  10001
#define SIZE_READ_BUFF  10001
#define SIZE_PIPE_BUFF  2001
#define PORT            33916
#define BACKLOG         10
#define MAX_PIPE_NUM    10000

#define FALSE           0
#define TRUE            1

#define IN              0
#define OUT             1

/*
 * Globals
 */
char send_buff[SIZE_SEND_BUFF];
char read_buff[SIZE_READ_BUFF];
char pipe_buff[SIZE_PIPE_BUFF];

char **argv;
int argc = 0;
int *pipe_map[MAX_PIPE_NUM];
int *old_pipe = NULL;

int connfd = 0;

/*
 * Pipe Map
 */
int *pipe_create(int p_n) {

    int *fd = malloc(sizeof(int) * 2);
    if(pipe(fd) < 0)    fprintf(stderr, "Fork failed\n");

    if(pipe_map[p_n])   old_pipe = pipe_map[p_n];
    else                old_pipe = NULL;

    pipe_map[p_n] = fd;
    return fd;

}

int pipe_get() {

    if(!pipe_map[0])    return 0;
    return pipe_map[0][IN];

}

void pipe_shift() {

    int i;
    for(i=0 ; i<(MAX_PIPE_NUM-1) ; i++) pipe_map[i] = pipe_map[i+1];

}

/*
 * Shell
 */
char **command_decode(char *command) {

    char *token = " \t\n\r";
    char *p = strtok(command, token);

    argc = 0;
    while(p) {
        argc ++;
        argv = realloc(argv, sizeof(char *) * argc);

        if(argv == NULL)    exit(EXIT_FAILURE);

        argv[argc-1] = p;
        p = strtok(NULL, token);
    }

    // for the last extra one
    argv = realloc(argv, sizeof(char *) * (argc+1));
    argv[argc] = NULL;

    return argv;

}

int prompt() {

    int r = 0;

    memset(send_buff, 0, sizeof(send_buff)); 
    memset(read_buff, 0, sizeof(read_buff)); 

    snprintf(send_buff, sizeof(send_buff), "$ ");
    write(connfd, send_buff, strlen(send_buff)); 
    r = read(connfd, read_buff, sizeof(read_buff));

    argv = command_decode(read_buff);
    if(strcmp(argv[0], "exit")==0) {
        close(connfd);
        return 0;   // same as end
    }

    return r;
}

void welcome_msg() {
    char *msg = "****************************************\n\
** Welcome to the information server. **\n\
****************************************\n";
    snprintf(send_buff, sizeof(send_buff), msg);
    write(connfd, send_buff, strlen(send_buff)); 
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

void debug_print_command(char** argv_s, int p_n) {
    int k;
    fprintf(stderr, "\n==========\n(pipe)exec: ");
    fprintf(stderr, "p_n = %d\n", p_n);
    for( k=0 ; argv_s[k]!=NULL ; k++ ){
        fprintf(stderr, ".%s", argv_s[k]);
    }
    fprintf(stderr, "\n");
}

void debug_fork_and_exec_last(int fd_in) {
    int i;
    printf("\n==========\n(rest)exec: ");
    for( i=0 ; argv[i]!=NULL ; i++ ){
        printf(".%s", argv[i]);
    }
    printf("\n");

    fprintf(stderr, "last...\n");
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[i]) fprintf(stderr, "pipe_map[%d] = %p, [%d][%d]\n", i, pipe_map[i], pipe_map[i][IN], pipe_map[i][OUT]);
    fprintf(stderr, "----\n");

    fprintf(stderr, "IN: %d\n", fd_in);
    fprintf(stderr, "OUT: -\n");
}

void debug_print_pipe_map() {
    int i;
    fprintf(stderr, "----\n");
    for( i=0 ; i<10 ; i++ )
        if(pipe_map[i]) fprintf(stderr, "pipe_map[%d] = %p, [%d][%d]\n", i, pipe_map[i], pipe_map[i][IN], pipe_map[i][OUT]);
    fprintf(stderr, "----\n");
}

/*
 * Handler
 */
// simple exec (no pipe included), for last command (no following pipe)
int fork_and_exec_last() {

    if(argc==0) return EXIT_SUCCESS;

    // bind pipe in
    int fd_in = pipe_get();
    if(fd_in)   dup2(fd_in, STDIN_FILENO);
    

    // bind out to stdout
    dup2(connfd, STDOUT_FILENO);    // duplicate socket on stdout

    if(DEBUG)   debug_fork_and_exec_last(fd_in);

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid ==0) {           // if child

        if( argv[0][0]=='/' || execvp(argv[0], argv)<0 ) {
            printf("Unknown command: [%s].\n", argv[0]);
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

// simple exec (no pipe included, but for piping OUTs)
int fork_and_exec_pipe(char **cmd, int p_n) {

    // create pipe
    int *fd = pipe_create(p_n);

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid ==0) {           // if child

        // output old_pipe content if exist
        if(old_pipe!=NULL) {

            memset(pipe_buff, 0, sizeof(pipe_buff)); 

            int count;
            if( (count = read(old_pipe[IN], pipe_buff, SIZE_PIPE_BUFF)) < 0 ) {
                fprintf(stderr, "read pipe connent error: %s\n", strerror(errno));
            }

            if(close(old_pipe[IN]) < 0) {
                fprintf(stderr, "pipe close error (old_pipe in): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            if(DEBUG)   fprintf(stderr, "cating [%d] -> [%d]\n", old_pipe[IN], fd[OUT]);
            if(DEBUG)   debug_print_pipe_cat_content(count);

            if( write(fd[OUT], pipe_buff, count) < 0 ) {
                fprintf(stderr, "write pipe connent error: %s\n", strerror(errno));
            }

        }

        // redirect STDOUT to pipe
        if(close(fd[IN]) < 0) {
            fprintf(stderr, "pipe close error (fd in): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        dup2(fd[OUT], STDOUT_FILENO);

        // DEBUG
        if(DEBUG)   debug_print_pipe_map();

        // redirect STDIN to pipe_map[0][IN]
        int fd_in = pipe_get();
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if(DEBUG)   fprintf(stderr, "IN: %d\nOUT: %d\n", fd_in, fd[OUT]);

        if( argv[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            printf("Unknown command: [%s].\n", cmd[0]);
            close(fd[OUT]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    } else {                        // if parent
        int return_val;    
        waitpid(pid, &return_val, 0);
        if( close(fd[OUT]) < 0 ) {
            fprintf(stderr, "pipe close error (fd out): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if( WEXITSTATUS(return_val) == EXIT_FAILURE )  return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int fork_and_exec_file(char **cmd, char *filepath) {

    pid_t pid;
    pid = fork();

    if(pid<0) {                     // if error
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid ==0) {           // if child

        // bind stdout to file
        int fd_file = open(filepath, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(fd_file, STDOUT_FILENO);

        // redirect STDIN to pipe_map[0][IN]
        int fd_in = pipe_get();
        if(fd_in)   dup2(fd_in, STDIN_FILENO);

        if( argv[0][0]=='/' || execvp(cmd[0], cmd)<0 ) {
            printf("Unknown command: [%s].\n", cmd[0]);
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

// helper tool
char **extract_command(int len) {

    int j;
    char **argv_s = malloc(sizeof(char *) * (len));

    for( j=0 ; j<argc ; j++ ) {
        // move to sub argv, which will be exec
        if( j<len ) {
            argv_s[j] = malloc(sizeof(char) * sizeof(argv[j]));
            strcpy(argv_s[j], argv[j]);
        }

        // shift argv
        if( j<(argc-len-1) ) {
            argv[j] = malloc(sizeof(char) * sizeof(argv[j+len+1]));
            strcpy(argv[j], argv[j+len+1]);
        }
    }

    argc -= (len+1);
    argv[argc] = NULL;
    argv_s[len] = NULL;

    return argv_s;
}

// a whole line as input (pipe may be included)
void command_handler() {

    int i;
    int is_pipe;

    while(1) {

        is_pipe = FALSE;

        for( i=0 ; i<argc ; i++ ) {
            if(argv[i] && argv[i][0] == '|') {

                int p_n = 1;
                if(strlen(argv[i]) == 1)  p_n = 1;
                else    sscanf(argv[i], "|%d", &p_n);

                is_pipe = TRUE;

                //
                char **argv_s = extract_command(i);

                if(DEBUG)   debug_print_command(argv_s, p_n);

                if(fork_and_exec_pipe(argv_s, p_n) == EXIT_FAILURE) {
                    return;
                }
                pipe_shift();

                break;

            }
            if(argv[i] && argv[i][0] == '>') {
                char *filepath = argv[i+1];
                char **argv_s = extract_command(i);
                if(!filepath)   fprintf(stderr, "filepath error\n");
                if( fork_and_exec_file(argv_s, filepath) == EXIT_FAILURE ) {
                    return;
                }
                return;
            }
        }
        if(!is_pipe)    break;

    }

    if(fork_and_exec_last() == EXIT_FAILURE)    return;
    pipe_shift();
}


// handle one socket connection
void client_handler() {

    if(DEBUG)   dup2(connfd, STDERR_FILENO);
    dup2(connfd, STDOUT_FILENO);

    // handle (first)
    welcome_msg(connfd);

    // handle (rest)
    while(1) {
        if(!prompt(connfd)) break;
        command_handler();
    }

}

void init_env() {
    system("export PATH=bin:.");
}


/*
 * Main
 */
int main(int argc, char *argv[]) {

    init_env();

    /* variables */
    int listenfd = 0;
    struct sockaddr_in serv_addr; 

    /* init */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    /* setup */
    serv_addr.sin_family = AF_INET;                 // internet protocols (IPv4)
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // address
    serv_addr.sin_port = htons(PORT);               // port

    /* socket - bind */
    int optval = 1;
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1 ) {
        perror("setsockopt");
    }
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    /* socket - listen */
    listen(listenfd, BACKLOG); 

    fprintf(stderr, "start listen on port %d...\n", PORT);
    while(1) {

        /* socket - accept */
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        fprintf(stderr, "accepted connection: %d\n", connfd);

        client_handler();

        /* socket - close */
        if(close(STDOUT_FILENO) < 0) {  // we've dup2 socket to sdtout
            fprintf(stderr, "close error: %s\n", strerror(errno));
        }
        fprintf(stderr, "closed connection: %d\n", connfd);
        sleep(1);

    }

    return 0;
}
