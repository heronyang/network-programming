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

#define PORT 1234
#define BACKLOG         10

char *argv[] = {"cat", NULL};

int main() {

    /* variables */
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    /* init */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    /* setup */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    /* socket - bind */
    int optval = 1;
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1 ) {
        perror("setsockopt");
    }
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    /* socket - listen */
    listen(listenfd, BACKLOG); 

    while(1) {

        /* socket - accept */
        int connfd;
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("accepted connection: %d\n", connfd);

        /* handling */
        dup2(connfd, STDIN_FILENO);
        dup2(connfd, STDOUT_FILENO);
        execvp(argv[0], argv);

        /* socket - close */
        if(close(connfd) < 0) {
            printf("close error: %s\n", strerror(errno));
        }
        printf("closed connection: %d\n", connfd);

        sleep(1);

    }


    return 0;
}
