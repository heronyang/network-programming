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
int g_shmid;

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

void shm_client_save(int shmid, struct sockaddr_in address) {

    fprintf(stderr, "save client: %d\n", child_count);

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    //
    char *default_name = "(no name)";
    shm[child_count].valid = TRUE;
    shm[child_count].pid = getpid();
    shm[child_count].name = malloc(sizeof(default_name)+1);
    strcpy(shm[child_count].name, default_name);
    shm[child_count].ip = inet_ntoa(address.sin_addr);
    shm[child_count].port = ntohs(address.sin_port);

    fprintf(stderr, "valid=%d, pid=%d, name=%s, ip=%s, port=%d\n", 
            shm[child_count].valid,
            shm[child_count].pid,
            shm[child_count].name,
            shm[child_count].ip,
            shm[child_count].port);

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
    signal(SIGINT, catch_int);

    /* SHM: init */
    int shmid;
    key_t key = SHM_KEY;

    int shm_size = sizeof(Client) * CLIENT_MAX_NUM;
    if ((shmid = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    g_shmid = shmid;    // for signal handler to release the shm space
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

            fprintf(stderr, "accepted connection: %d\n", connfd);

            shm_client_save(shmid, serv_addr);
            broadcast_new_comer(serv_addr);
            client_handler(connfd);

            /* socket - close */
            if(close(connfd) < 0) {
                perror("close");
            }
            fprintf(stderr, "closed connection: %d\n", connfd);

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
