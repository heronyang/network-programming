#ifndef CLIENTS_H
#define CLIENTS_H

#include <netinet/in.h>
#include "mytype.h"

void clients_init();
void clients_new(struct sockaddr_in address, int socket);
void clients_close(int socket);
Client *clients_get(int client_id);
Client *clients_get_from_socket(int socket);
int clients_get_id_from_socket(int socket);

#endif
