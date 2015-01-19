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

int contain_prompt() {
    int i;
    for( i=0 ; i<strlen(buf) ; i++ ) {
        if(buf[i] == '%')   return 1;
    }
    return 0;
}

/* Socket Function */
int write_command_init(int ind) {
    char buf[BUFSIZE];
    char filepath[MAX_PATH_LENGTH] = FILES_PATH_DIR;    // default dir
    strcat(filepath, req[ind].file);
    req[ind].fp = fopen(filepath, "r");
    if(req[ind].fp == NULL) {
        strcpy(buf, "wrong file<br />");
        write_content_at(ind, buf, 0);
        return 1;
    }

    return 0;
}

void write_command_close(int ind) {
    fclose(req[ind].fp);
}

void write_command_next(int ind) {

    int n;

    if(!fgets(buf, BUFSIZE, req[ind].fp)) {
        error("fgets");
    }
    write_content_at(ind, wrap_html(buf), 1);

    if(buf[0] == '\n')  return;
    fprintf(stderr, "%s(%d)\n", buf, (int)strlen(buf));
    n = write(req[ind].socket, buf, strlen(buf));
    if(n < 0)   error("ERROR writing to socket");

}


void bash_new(int ind) {

    int sockfd;
    struct sockaddr_in serveraddr;
    struct hostent *server;

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    /* get server */
    char buf[BUFSIZE];
    server = gethostbyname(req[ind].sh);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", IP);

        // TODO: hostname
        strcpy(buf, "wrong hostname<br />");
        write_content_at(ind, buf, 0);

        return;
    }

    /* setup socket */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(atoi(req[ind].sp));
    
    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0) {
        error("ERROR connecting");
        return;
    }

    /* send SOCKS request */
    char package[8];
    struct hostent *sh;
    package[0] = 4;
    package[1] = 1;
    package[2] = atoi(req[ind].port)/256;
    package[3] = atoi(req[ind].port)%256;
    sh = gethostbyname(req[ind].sh);
    fprintf(stderr, "sh->> %u\n", (unsigned char)sh->h_addr_list[0][0]);
    fprintf(stderr, "sh->> %u\n", (unsigned char)sh->h_addr_list[0][1]);
    fprintf(stderr, "sh->> %u\n", (unsigned char)sh->h_addr_list[0][2]);
    fprintf(stderr, "sh->> %u\n", (unsigned char)sh->h_addr_list[0][3]);
    package[4] = (unsigned char)sh->h_addr_list[0][0];
    package[5] = (unsigned char)sh->h_addr_list[0][1];
    package[6] = (unsigned char)sh->h_addr_list[0][2];
    package[7] = (unsigned char)sh->h_addr_list[0][3];
    if( write(sockfd, package, sizeof(package))<0 ) {
        perror("sock request write");
    }

    if( read(sockfd, package, sizeof(package)) <0 ) {
        perror("read");
    }

    if( package[1] == 91 ) {
        perror("return error");
    }

    /* save socket */
    req[ind].socket = sockfd;

}

void bash_serve() {

    int i, s, max_s, n;
    int activity;
    struct timeval timeout; // second, microsecs
    fd_set fds;

    timeout.tv_sec = 600;
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

        if(!max_s) {
            fprintf(stderr, "no more existing socket\n");
            break;
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
            //n = recv(s, buf, BUFSIZE, 0);
            n = read(s, buf, BUFSIZE);

            if(n<=0) {
                fprintf(stderr, "close socket (%d): %d\n", i+1, s);
                close(s);
                req[i].socket = 0;
                write_command_close(i);
                FD_CLR(s, &fds);
            }

            if(DEBUG)   printf("<p>Read from server(%d):<br />%s***END***<br /></p>", i+1, wrap_html(buf));
            write_content_at(i, wrap_html(buf), 0);

            // write next command
            //if(buf[0] == '%' || (buf[0] == '*' && buf[3] == '*')) {
            if(contain_prompt()) {
                fprintf(stderr, ">>> socket (%d) get:%s\nsend:\n<<<\n", i+1, buf);
                write_command_next(i);
                fprintf(stderr, "\n<<<\n");
            }

        }

    }

}


/* Main */
void rbs() {

    int i;
    char buf[BUFSIZE];
    for( i=0 ; i<MAX_REQUEST ; i++ ) {
        Request r = req[i];
        if( !(r.ip && r.file) )   continue;
        if( write_command_init(i) == 1 )    continue;
        if( r.port==NULL ) {
            strcpy(buf, "wrong ip<br />");
            write_content_at(i, buf, 0);
            fprintf(stderr, "ip not found\n");
            continue;
        }
        bash_new(i);
    }

    bash_serve();

    return;

}
