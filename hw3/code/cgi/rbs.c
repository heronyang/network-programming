#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <errno.h>

#include "constant.h"
#include "type.h"
#include "html_client.h"
#include "rbs.h"

#define IP          "127.0.0.1"
#define PORT        33917

/* External */
Request req[MAX_REQUEST];

/* Global */
char buf[BUFSIZE];

/* Tool Function */
void error(char *msg) {
    perror(msg);
    exit(0);
}

/* Socket Function */
void bash_new(int ind) {

    char filepath[MAX_PATH_LENGTH] = FILES_PATH_DIR;    // default dir

    int sockfd, fd, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;

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

    /* save socket */
    req[ind].socket = sockfd;

    /* write file content */
    // 1. read from file
    strcat(filepath, req[ind].file);
    fd = open(filepath, O_RDONLY);
    if(fd < 0)  error("open");
    bzero(buf, BUFSIZE);
    n = read(fd, buf, BUFSIZE);
    if(n < 0)   error("ERROR reading from file");
    close(fd);

    // 2. write to socket
    n = write(sockfd, buf, strlen(buf));
    if(n < 0)   error("ERROR writing to socket");

}

void bash_serve() {

    int i, s, max_s, n;
    int activity;
    struct timeval timeout; // second, microsecs
    fd_set fds;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    while(1) {

        //
        FD_ZERO(&fds);
        max_s = 0;

        for( i=0 ; i<MAX_REQUEST ; i++ ) {

            s = req[i].socket;
            if(!s) continue;
            if(s>max_s) max_s = s;

            FD_SET(s, &fds);
            fprintf(stderr, "socket (%d)%d is set\n", i+1, s);

        }

        // select
        activity = select(max_s+1, &fds, NULL, NULL, &timeout);
        if( (activity<0) && (errno!=EINTR) ) {
            error("select");
        } else if( activity == 0 ) {
            fprintf(stderr, "timeout\n");
            break;
        }

        for( i=0 ; i<MAX_REQUEST ; i++ ) {

            s = req[i].socket;
            if(!s) continue;
            if(!FD_ISSET(s, &fds))  continue;

            // read
            bzero(buf, BUFSIZE);
            n = recv(s, buf, BUFSIZE, 0);
            if(n<=0) {
                fprintf(stderr, "close socket (%d): %d\n", i+1, s);
                close(s);
                req[i].socket = 0;
                FD_CLR(s, &fds);
            }

            printf("<p>Read from server(%d):<br />%s***END***<br /></p>", i+1, buf);
            write_content_at(i, wrap_html(buf));
            fprintf(stderr, "Read from server(%d):<br />%s***END***<br />", i+1, buf);

        }

    }

    // FIXME: when to close socket?
}


/* Main */
void rbs() {

    int i;
    for( i=0 ; i<MAX_REQUEST ; i++ ) {
        Request r = req[i];
        if( !(r.ip && r.port && r.file) )   continue;
        bash_new(i);
    }

    bash_serve();

    return;

}
