/*
 * Network Programming - HW4
 * Author:      Heron Yang
 * Director:    I-Chen Wu (Prof.)
 * Date:        Fri Jan  2 14:34:48 CST 2015
 *
 * SOCKS Proxy Server
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#include "constant.h"

/* Globals */
int child_count = 0;

/* 
 * Signal Handlers 
 */
void catch_chld(int snum) {

    int pid;
    int status;

    pid = wait(&status);
    fprintf(stderr, "parent: child process pid=%d exited with value %d\n", pid, WEXITSTATUS(status));

    if(pid != -1)   child_count --;

}

void catch_int(int i) {

    if(child_count!=0) {
        fprintf(stderr, "unfinished client, can't close\n");
        // return;
    }

    // release shared memory from system
    exit(0);    // end program

}

/*
 * Tools
 */
char *get_port_str(char *buf) {
    static char res[PORT_STR];
    unsigned int port = ((buf[2] & BYTE) << 8) + (buf[3] & BYTE);
    sprintf(res, "%u", port);
    printf("port >> %s\n", res);
    return res;
}

char *get_ip_str(char *buf) {
    static char res[IP_STR];
    sprintf(res, "%u.%u.%u.%u",
            buf[4] & BYTE,
            buf[5] & BYTE,
            buf[6] & BYTE,
            buf[7] & BYTE);
    printf("ip >> %s\n", res);
    return res;
}

int connectTCP(const char *host, const char *port) {

    struct sockaddr_in serv_addr;
    int sock;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));

    if(inet_pton(AF_INET, host, &serv_addr.sin_addr) < 0){
        perror("inet_pton");
        return -1;
    }

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return -1;
    }

    printf("connected to remote server: %s:%s\n", host, port);

    return sock;

}

int dual_forwarding(int sock, int rsock) {

    int n, is_close_sock = 0, is_close_rsock = 0, i;
    int max_fd = (sock>rsock)? sock : rsock;
    max_fd = getdtablesize();

    char read_buf[BUF_SIZE];

    fd_set fds, _fds;

    FD_ZERO(&_fds);
    FD_SET(sock, &_fds);
    FD_SET(rsock, &_fds);

    printf("sock=%d\trsock=%d\n", sock, rsock);

    while(1) {

        memcpy(&fds, &_fds, sizeof(_fds));

        if( select(max_fd, &fds, NULL, NULL, NULL)<0 && (errno!=EINTR) ) {
            perror("select");
            return -1;
        }

        if( FD_ISSET(sock, &fds) ) {    // master
            memset(read_buf, 0, BUF_SIZE);
            n = recv(sock, read_buf, BUF_SIZE, 0);
            if( n<0 ) {
                perror("recv");
                return -1;
            } else if( n==0 ) {

                printf("pid=%d: end sock\n", getpid());

                is_close_sock = 1;
                FD_CLR(sock, &fds);
                shutdown(sock, SHUT_RD);
                shutdown(rsock, SHUT_RD);

            } else {
                printf("%d: read from master:\n%s\n", getpid(), read_buf);
                if( send(rsock, read_buf, n, 0)<0 ) {
                    perror("send");
                    return -1;
                } else {
                    printf("%d: send to remote:\n%s\n", getpid(), read_buf);
                }
                fflush(stdout);
            }
        }

        if( FD_ISSET(rsock, &fds) ) {   // remote server
            memset(read_buf, 0, BUF_SIZE);
            n = recv(rsock, read_buf, BUF_SIZE, 0);
            if( n<0 ) {
                perror("recv");
                return -1;
            } else if( n==0 ) {

                printf("pid=%d: end rsock\n", getpid());

                is_close_rsock = 1;
                FD_CLR(rsock, &fds);
                shutdown(sock, SHUT_RD);
                shutdown(rsock, SHUT_RD);

            } else {
                printf("%d: read from remote:%s\n", getpid(), read_buf);
                if( send(sock, read_buf, n, 0)<0 ) {
                    perror("send");
                    return -1;
                } else {
                    printf("%d: send from master:%s\n", getpid(), read_buf);
                }
                fflush(stdout);
            }
        }

        if(is_close_sock && is_close_rsock) {
            break;
        }

    }
    printf("client handler end\n");

    close(sock);
    close(rsock);
    return 0;
}

/*
 * Client
 */
void client_handler(int sock) {

    char read_buf[BUF_SIZE];
    char write_buf[BUF_SIZE];

    if( read(sock, read_buf, BUF_SIZE) <= 0 ) {
        fprintf(stderr, "[client_handler] read failed\n");
        return;
    }

    unsigned char VN = read_buf[0] ;
    unsigned char CD = read_buf[1] ;
    // unsigned int dst_port = read_buf[2] << 8 | read_buf[3] ;
    // unsigned int dst_ip = read_buf[4] << 24 | read_buf[5] << 16 | read_buf[6] << 8 | read_buf[7] ;
    char *USER_ID = read_buf + 8 ;
    
    char *str_dst_port = get_port_str(read_buf);
    char *str_dst_ip = get_ip_str(read_buf);

    // print
    printf("\nread (size: %d)>> %s\n", (int)strlen(read_buf), read_buf);
    int i;
    for( i=0 ; i<strlen(read_buf) ; i++ ) {
        printf("[%d]: (%d) %c\t", i, read_buf[i], read_buf[i]);
        if( (i+1) % 10 == 0)    printf("\n");
    }
    printf("VN=%d, CD=%d, USER_ID=%s\n", VN, CD, USER_ID);

    int rsock;

    if( CD == 1 ) {         // CONNECT

        printf("CD = 1 (get CONNECT)\n");

        write_buf[0] = 0;
        write_buf[1] = (unsigned char) 90;
        for( i=2 ; i<=7 ; i++ ) {
            write_buf[i] = read_buf[i];
        }

        // debug
        for( i=0 ; i<8 ; i++ ) {
            printf("read_buf[%d]: %u\n", i, read_buf[i]&0x000000FF);
        }
        for( i=0 ; i<8 ; i++ ) {
            printf("write_buf[%d]: %u\n", i, write_buf[i]&0x000000FF);
        }

        rsock = connectTCP(str_dst_ip, str_dst_port);
        if(rsock <= 0) {
            perror("connectTCP");
            write_buf[1] = (unsigned char) 91;
        }

        if( write(sock, write_buf, 8)<0 ) {
            perror("write reply");
        }
        dual_forwarding(sock, rsock);    // blocking

    } else if( CD == 2 ) {  // BIND
    }

}

/*
 * Main
 */
int main(int argc, char *argv[]) {

    /* variables */
    int listenfd = 0, addrlen;
    struct sockaddr_in serv_addr; 

    /* SIGNAL: catch control c */
    if (signal(SIGINT, catch_int) == SIG_ERR)
        fprintf(stderr, "can't catch SIGINT\n");
    /* SIGNAL: detect child termination */
    if (signal(SIGCHLD, catch_chld) == SIG_ERR)
        fprintf(stderr, "can't catch SIGINT catch_chld\n");

    /* init */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    /* setup */
    serv_addr.sin_family = AF_INET;                 // internet protocols (IPv4)
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // address
    serv_addr.sin_port = htons(PORT);               // port

    /* socket - bind */
    int optval = 1;
    if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1 ) {
        perror("setsockopt");
    }
    if( bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) {
        perror("bind");
    }

    /* socket - listen */
    listen(listenfd, BACKLOG); 

    addrlen = sizeof(serv_addr);

    fprintf(stderr, "start listen on port %d...\n", PORT);
    while(1) {

        /* socket - accept */
        int connfd = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen); 
        if( connfd<0 ) {
            perror("accept");
        }
        child_count ++;

        /* fork to handle */
        pid_t pid;
        pid = fork();

        if(pid<0) {                     // if error

            fprintf(stderr, "Fork failed\n");
            exit(EXIT_FAILURE);

        } else if (pid ==0) {           // if child

            // client: init
            fprintf(stderr, "accepted connection: %d\n", connfd);

            // client: handle
            client_handler(connfd);

            // client: close
            if(close(connfd) < 0) {
                perror("close");
            }

            exit(EXIT_SUCCESS);

        } else {

            if(close(connfd) < 0) {
                perror("close");
            }
            fprintf(stderr, "closed connection: %d\n", connfd);

        }

    }

    return 0;
    //
}
