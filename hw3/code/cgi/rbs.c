#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

#define BUFSIZE 1024

#define IP          "127.0.0.1"
#define PORT        33917
#define FILEPATH    "test.txt"

/* Tool Function */
void error(char *msg) {
    perror(msg);
    exit(0);
}

/* Main */
int main(int argc, char **argv) {

    int sockfd, fd, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buf[BUFSIZE];

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    /* get server */
    server = gethostbyname(IP);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", IP);
        exit(0);
    }

    /* setup socket */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(PORT);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0) {
        error("ERROR connecting");
    }

    // 1. read from socket
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if(n < 0)   error("ERROR reading from socket");
    printf("Read from server: %s***END***", buf);

    // 2. read from file
    fd = open("test.txt", O_RDONLY);
    if(fd < 0)  error("open");
    bzero(buf, BUFSIZE);
    n = read(fd, buf, BUFSIZE);
    if(n < 0)   error("ERROR reading from file");

    // 3. write to socket
    n = write(sockfd, buf, strlen(buf));
    if(n < 0)   error("ERROR writing to socket");

    // 4. read socket
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if(n < 0)   error("ERROR reading from socket");

    //
    printf("Echo from server: %s***END***", buf);
    close(sockfd);

    //
    return 0;

}
