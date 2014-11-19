#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <arpa/inet.h>    //close
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "constant.h"
#include "mytype.h"

int client_socket;
int shmid_msg;

/* [Private] Send */
void broadcast_sender_pid(int pid) {
    kill(pid, SIGUSR1);
}

void broadcast_sender_all(int shmid) {
    int i;
    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(shm[i].valid) {
            broadcast_sender_pid(shm[i].pid);
        }
    }
    shmdt(shm);
}

/* [Public] Recieve (Signal Callback) */
void broadcast_catch(int signo) {

    char *msg;
    if ((msg = shmat(shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    if (signo == SIGUSR1) {
        fprintf(stderr, "get SIGUSR1: %s\n", msg);
        if(write(client_socket, msg, strlen(msg)) < 0)
            perror("write");
    }
    // else, drop

    shmdt(msg);
}

/* [Public] Events */
void broadcast_init(int connfd, int sm) {
    client_socket = connfd;
    fprintf(stderr, "init socket:%d\n", client_socket);

    shmid_msg = sm;
}
void broadcast_user_connect(int shmid, struct sockaddr_in address) {

    fprintf(stderr, "broadcast connect\n");

    char *msg;
    if ((msg = shmat(shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    sprintf(msg, " *** User '(no name)' entered from %s/%d. ***\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    shmdt(msg);

    broadcast_sender_all(shmid);

}

void broadcast_user_disconnect(int shmid) {

    fprintf(stderr, "broadcast disconnect\n");

    char *msg;
    if ((msg = shmat(shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    int i, pid = getpid();
    char name[NAME_SIZE];
    char *t_name = "(no name)";
    strcpy(name, t_name);
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(shm[i].valid && shm[i].pid == pid) {
            strcpy(name, shm[i].name);
        }
    }

    sprintf(msg, " *** User '%s' left. ***\n", name);

    shmdt(msg);
    shmdt(shm);

    broadcast_sender_all(shmid);

}
