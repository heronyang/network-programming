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

#define SIZE_SEND_BUFF  1025
#define SIZE_READ_BUFF  1025
#define PORT            33916
#define BACKLOG         10

char send_buff[SIZE_SEND_BUFF];
char read_buff[SIZE_READ_BUFF];

char **argv;    // decoded command

/*
 * Shell
 */
char **command_decode(char *command) {
    char *token = " \t\n\r";                // characters for splitting
    char *p = strtok(command, token);
    int argc = 0;

    while(p) {
        argc ++;
        argv = realloc(argv, sizeof(char *) * argc);

        // if memory allocation failed
        if(argv == NULL)    exit(-1);

        argv[argc-1] = p;
        p = strtok(NULL, token);
    }

    // for the last extra one
    argv = realloc(argv, sizeof(char *) * (argc+1));
    argv[argc] = 0;     // set last one as NULL

    return argv;
}

int prompt(int connfd) {

    int r = 0;

    snprintf(send_buff, sizeof(send_buff), "$ ");
    write(connfd, send_buff, strlen(send_buff)); 
    r = read(connfd, read_buff, sizeof(read_buff));

    argv = command_decode(read_buff);
    if(strcmp(argv[0], "exit")==0) {
        close(connfd);
        return 0;   // same as end
    }

    printf("[prompt] r = %d\n", r);
    return r;

}

void command_handler() {
    execvp(argv[0], argv);
}

void client_handler(int connfd) {
    // init
    memset(send_buff, 0, sizeof(send_buff)); 
    memset(read_buff, 0, sizeof(read_buff)); 

    // handle (first)
    if(!prompt(connfd)) return;

    // handle (rest)
    while(1) {
        pid_t pid;
        pid = fork();
        
        if(pid<0) {                     // if error
            fprintf(stderr, "Fork failed\n");
            exit(-1);
        } else if (pid ==0) {           // if child

            dup2(connfd, STDOUT_FILENO);    // duplicate socket on stdout
            dup2(connfd, STDERR_FILENO);    // duplicate socket on stderr
            close(connfd);                  // close connection

            command_handler();
            exit(0);

        } else {                        // if parent
            wait(NULL);
            if(!prompt(connfd)) {
                close(connfd);
                return;
            }
        }
    }
}


/*
 * Main
 */
int main(int argc, char *argv[]) {

    /* variables */
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    time_t ticks; 

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

    printf("start listen on port %d...\n", PORT);
    while(1) {

        /* socket - accept */
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("accepted connection: %d\n", connfd);

        client_handler(connfd);

        printf("closed connection: %d\n", connfd);
        sleep(1);

    }

    return 0;
}
