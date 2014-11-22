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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <arpa/inet.h>

#include "constant.h"
#include "mytype.h"
#include "variable.h"
#include "signal.h"
#include "client.h"
#include "clients.h"

/*
 * Main
 */
int main(int argc, char *argv[]) {

    /* =============================================== */
    /* variables */
    /* =============================================== */
    int connfd, listenfd = 0, addrlen;
    int max_fd, activity, cs, i;
    struct sockaddr_in serv_addr; 
    fd_set fds;
    Client *c;

    /* =============================================== */
    /* init */
    /* =============================================== */

    /* signal: init */
    // signal_init();

    /* client: init */
    clients_init();

    /* FIFO: init */
    /*
    fifo_init();
    fifo_lock_init();
    */

    /* =============================================== */
    /* socket */
    /* =============================================== */

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

    addrlen = sizeof(serv_addr);


    /* =============================================== */
    fprintf(stderr, "start listen on port %d...\n", PORT);
    while(1) {

        /* FDS */
        FD_ZERO(&fds);

        // add current socket to fds
        FD_SET(listenfd, &fds);
        max_fd = listenfd;

        // add other client sockets to fds
        for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

            c = clients_get(i);
            cs = c->socket;

            if( cs>0 )          FD_SET(cs, &fds);
            if( cs > max_fd )   max_fd = cs;

        }

        // select
        activity = select( max_fd+1, &fds, NULL, NULL, NULL );
        if( (activity<0) && (errno!=EINTR) ) {
            perror("select error");
        }

        if( FD_ISSET(listenfd, &fds) ) {  // new client

            /* socket - accept */
            connfd = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen); 

            fprintf(stderr, "accepted connection: %d\n", connfd);

            clients_new(serv_addr, connfd);

            welcome_msg(connfd);
            //broadcast_user_connect(serv_addr);

            print_prompt_sign(connfd);

        }

        for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

            c = clients_get(i);
            cs = c->socket;

            if( !FD_ISSET(cs, &fds) )   continue;

            // client: handle
            if( client_handler(cs) == CLIENT_END ) {

                //fifo_close();
                //broadcast_user_disconnect();
                clients_close(connfd);

                fprintf(stderr, "closed connection: %d\n", connfd);

            }
        

        }

    }

    return 0;
    //
}
