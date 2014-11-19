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
#include <sys/wait.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>    //close

#include "constant.h"
#include "pipe.h"
#include "client.h"
#include "mytype.h"
#include "broadcast.h"

int child_count = 0;
int g_shmid, g_shmid_msg;

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
        return;
    }

    // release shared memory from system
    shmctl(g_shmid, IPC_RMID, NULL);
    shmctl(g_shmid_msg, IPC_RMID, NULL);
    exit(0);    // end program

}

/*
 * SHM
 */
void shm_init(int shmid) {

    int i;

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        shm[i].valid = FALSE;
    }
    shmdt(shm);

}

void shm_client_new(int shmid, struct sockaddr_in address, int socket) {

    int client_id = child_count - 1;
    fprintf(stderr, "client #: %d\n", client_id);

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    //
    char *default_name = "(no name)";
    shm[client_id].valid = TRUE;
    shm[client_id].pid = getpid();
    shm[client_id].name = malloc(sizeof(default_name)+1);
    strcpy(shm[client_id].name, default_name);
    shm[client_id].ip = inet_ntoa(address.sin_addr);
    shm[client_id].port = ntohs(address.sin_port);
    shm[client_id].socket = socket;

    fprintf(stderr, "valid=%d, pid=%d, name=%s, ip=%s, port=%d\n", 
            shm[client_id].valid,
            shm[client_id].pid,
            shm[client_id].name,
            shm[client_id].ip,
            shm[client_id].port);

    shmdt(shm);

}

void shm_client_delete(int shmid) {

    int pid = getpid();

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(shm[i].valid && shm[i].pid == pid) {
            shm[i].valid = FALSE;
        }
    }

    shmdt(shm);

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
    if (signal(SIGUSR1, broadcast_catch) == SIG_ERR)
        fprintf(stderr, "can't catch SIGUSR1\n");

    /* SHM: init */
    int shmid, shmid_msg;
    key_t key = SHM_KEY, msg_key = SHM_MSG_KEY;

    int shm_size = sizeof(Client) * CLIENT_MAX_NUM;
    if ((shmid = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    g_shmid = shmid;    // for signal handler to release the shm space

    int shm_size_msg = sizeof(char) * MESSAGE_SIZE;
    if ((shmid_msg = shmget(msg_key, shm_size_msg, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    g_shmid_msg = shmid_msg;
    shm_init(shmid);

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

    fprintf(stderr, "start listen on port %d...\n", PORT);
    while(1) {

        /* socket - accept */
        int connfd = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen); 
        child_count ++;

        /* fork to handle */
        pid_t pid;
        pid = fork();

        /* detect child termination */
        signal(SIGCHLD, catch_chld);

        if(pid<0) {                     // if error

            fprintf(stderr, "Fork failed\n");
            exit(EXIT_FAILURE);

        } else if (pid ==0) {           // if child

            shm_client_new(shmid, serv_addr, connfd);
            broadcast_init(connfd, shmid_msg);

            fprintf(stderr, "accepted connection: %d\n", connfd);
            broadcast_user_connect(shmid, serv_addr);

            client_handler(connfd, shmid);

            /* socket - close */
            if(close(connfd) < 0) {
                perror("close");
            }
            fprintf(stderr, "closed connection: %d\n", connfd);
            broadcast_user_disconnect(shmid);
            shm_client_delete(shmid);

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
