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

/*
 * Shell
 */
void command_handler(char *command) {
    execl(command, NULL, NULL, NULL);
}

void client_handler(int connfd) {
    // init
    int rc = 0;
    char send_buff[SIZE_SEND_BUFF];
    char read_buff[SIZE_READ_BUFF];
    memset(send_buff, '0', sizeof(send_buff)); 

    // handle
    snprintf(send_buff, sizeof(send_buff), "$ ");
    write(connfd, send_buff, strlen(send_buff)); 

    rc = read(connfd, read_buff, sizeof(read_buff));
    command_handler(read_buff);
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


        /* socket - close */
        close(connfd);
        printf("closed connection: %d\n", connfd);
        sleep(1);

    }
}
