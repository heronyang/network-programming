#include <stdio.h>
#include <arpa/inet.h>    //close
#include "constant.h"

char message[MESSAGE_SIZE];

/* Events */
void broadcast_new_comer(struct sockaddr_in serv_addr) {
    fprintf(stderr, "broadcast is called\n");
}

/* Send */
void broadcast_sender_all(int shmid) {
}

void broadcast_sender_pid(int pid) {
}
