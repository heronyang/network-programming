#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <arpa/inet.h>    //close
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "constant.h"
#include "mytype.h"
#include "variable.h"
#include "client_name.h"

int client_socket;
int g_shmid, g_shmid_msg;

/* [Private] Send */
void broadcast_sender_pid(int pid) {
    kill(pid, SIGUSR1);
}

void broadcast_sender_all() {
    int i;
    Client *shm;
    if ((shm = shmat(g_shmid, NULL, 0)) == (Client *) -1) {
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

/* Tool Function */
char *get_my_name() {

    Client *shm;
    if ((shm = shmat(g_shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    int i, pid = getpid();
    char *name = malloc(sizeof(char) * NAME_SIZE);
    char *t_name = "(no name)";
    strcpy(name, t_name);
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(shm[i].valid && shm[i].pid == pid) {
            strcpy(name, getname(i));
        }
    }

    shmdt(shm);

    return name;
}

char *get_following(char *buff) {

    int i, start = FALSE, len = strlen(buff);
    char *res = malloc(sizeof(char) * SIZE_READ_BUFF);
    for( i=0 ; i<len ; i++ ) {
        if( buff[i]!=' ' )  start = TRUE;
        if( start && buff[i]==' ' ) break;
    }

    if(buff[len-1] == '\n') buff[len-1] = '\0';

    memcpy(res, &buff[i+1], strlen(buff) - i);

    return res;

}

int get_pid_from_client_id(int client_id) {

    Client *shm;
    if ((shm = shmat(g_shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    int r = 0;

    if(shm[client_id].valid)    r = shm[client_id].pid; 
    shmdt(shm);

    return r;

}

int check_client_exist(int client_id) {

    Client *shm;
    if ((shm = shmat(g_shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    if(shm[client_id].valid)    return TRUE;
    return FALSE;

}

/* [Public] Recieve (Signal Callback) */
void broadcast_catch(int signo) {

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
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
void broadcast_init(int connfd) {
    client_socket = connfd;
    fprintf(stderr, "init socket:%d\n", client_socket);
}

void broadcast_user_connect(struct sockaddr_in address) {

    fprintf(stderr, "broadcast connect\n");

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    sprintf(msg, " *** User '(no name)' entered from %s/%d. ***\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    shmdt(msg);

    broadcast_sender_all();

}

void broadcast_user_disconnect() {

    fprintf(stderr, "broadcast disconnect\n");

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    sprintf(msg, "*** User '%s' left. ***\n", get_my_name());

    shmdt(msg);

    broadcast_sender_all();

}

void broadcast_cmd_name() {

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    Client *shm;
    if ((shm = shmat(g_shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }

    int pid = getpid(), i;
    int port = 0;
    char ip[IP_STRING_SIZE], name[NAME_SIZE];
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(shm[i].valid && shm[i].pid == pid) {
            strcpy(ip, shm[i].ip);
            port = shm[i].port;
            strcpy(name, getname(i));
        }
    }


    sprintf(msg, "*** User from %s/%d is named '%s'. ***\n", ip, port ,name);

    shmdt(shm);
    shmdt(msg);

    broadcast_sender_all();

}

void broadcast_cmd_yell(char *buff) {

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    
    sprintf(msg, "*** %s yelled ***: %s\n", get_my_name(), get_following(buff));

    shmdt(msg);

    broadcast_sender_all();

}

void broadcast_cmd_tell(int target_id, char *buff) {

    char *msg;
    if ((msg = shmat(g_shmid_msg, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    
    // strip twice to remove first two words
    sprintf(msg, "*** %s told you ***: %s\n", get_my_name(), get_following(get_following(buff)));

    shmdt(msg);

    broadcast_sender_pid(get_pid_from_client_id(target_id));

}
