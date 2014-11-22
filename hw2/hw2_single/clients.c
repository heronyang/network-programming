#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "variable.h"
#include "constant.h"

#include "clients.h"

/* External Globals */
int client_count;
Client clients[CLIENT_MAX_NUM];

/*
 * clients
 */
void clients_init() {

    client_count = 0;

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        clients[i].valid = FALSE;
    }

}

void clients_new(struct sockaddr_in address, int socket) {

    client_count ++;

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        if(clients[i].valid == FALSE) {

            clients[i].valid = TRUE;
            clients[i].ip = inet_ntoa(address.sin_addr);
            clients[i].port = ntohs(address.sin_port);
            clients[i].socket = socket;

            strcpy(clients[i].name, "(no name)");

            return;

        }

    }

}

void clients_close(int socket) {

    client_count --;

    Client *c = clients_get_from_socket(socket);
    if(close(c->socket) < 0) {
        perror("close");
    }
    c->valid = FALSE;

}

Client *clients_get(int client_id) {

    if( client_id >=0 && client_id < CLIENT_MAX_NUM )   return &clients[client_id];
    return NULL;

}

Client *clients_get_from_socket(int socket) {

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        if(clients[i].valid == TRUE && clients[i].socket == socket) return &clients[i];

    }

    return NULL;

}

int clients_get_id_from_socket(int socket) {

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        if(clients[i].valid == TRUE && clients[i].socket == socket) return i;

    }

    return 0;

}
