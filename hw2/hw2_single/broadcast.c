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
#include "clients.h"
#include "broadcast.h"

char send_buff[SIZE_SEND_BUFF];

/* Private */
void broadcast_client(int client_id, char *content) {

    int connfd;
    Client *c;
    c = clients_get(client_id);
    connfd = c->socket;

    write(connfd, content, strlen(content)); 

}

void broadcast_all(char *content) {

    int i;
    Client *c;

    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        c = clients_get(i);
        if(c->valid) {
            fprintf(stderr, "broadcasting to client_id = %d\n", i);
            broadcast_client(i, content);
        }
    }

}


/* Tool Function */
/*
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
*/

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

int check_client_exist(int client_id) {

    return clients[client_id].valid;

}

/* [Public] Recieve (Signal Callback) */
/* [Public] Events */
void broadcast_user_connect(int connfd, struct sockaddr_in address) {

    fprintf(stderr, "broadcast connect\n");
    sprintf(send_buff, "*** User '(no name)' entered from %s/%d. ***\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    broadcast_all(send_buff);

}

void broadcast_user_disconnect(int connfd) {

    Client *c = clients_get_from_socket(connfd);

    fprintf(stderr, "broadcast disconnect\n");
    sprintf(send_buff, "*** User '%s' left. ***\n", c->name);

    broadcast_all(send_buff);

}

void broadcast_cmd_name(int connfd) {

    int port = 0;
    char ip[IP_STRING_SIZE], name[NAME_SIZE];

    Client *c = clients_get_from_socket(connfd);
    strcpy(ip, c->ip);
    port = c->port;
    strcpy(name, c->name);


    sprintf(send_buff, "*** User from %s/%d is named '%s'. ***\n", ip, port ,name);

    broadcast_all(send_buff);

}

void broadcast_cmd_yell(int connfd, char *buff) {

    Client *c = clients_get_from_socket(connfd);

    sprintf(send_buff, "*** %s yelled ***: %s\n", c->name, get_following(buff));

    broadcast_all(send_buff);

}

void broadcast_cmd_tell(int connfd, int target_id, char *buff) {

    Client *c = clients_get_from_socket(connfd);

    // strip twice to remove first two words
    sprintf(send_buff, "*** %s told you ***: %s\n", c->name, get_following(get_following(buff)));

    broadcast_client(target_id, send_buff);

}

void broadcast_cmd_fifo_in(int connfd, int source_id, char *cmd) {
    // *** (my name) (#<my client id>) just received from (other client's name) (#<other client's id>) by '(command line)' ***
    // ex. *** IamUser (#3) just received from student7 (#7) by 'cat <7' ***

    Client *c = clients_get_from_socket(connfd);
    Client *s_c = clients_get(source_id);
    int client_id = clients_get_id_from_socket(connfd);
    
    int len = strlen(cmd);
    if(cmd[len-1] == '\n')  cmd[len-1] = '\0';

    // strip twice to remove first two words
    sprintf(send_buff, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", c->name, client_id+1, s_c->name, source_id+1, cmd);

    broadcast_all(send_buff);
}

void broadcast_cmd_fifo_out(int connfd, int target_id, char *cmd) {
    // *** (name) (#<client id>) just piped '(command line)' to (receiver's name) (#<receiver's client_id>) ***
    // ex. *** IamUser (#3) just piped 'cat test.html | cat >1' to Iam1 (#1) ***

    Client *c = clients_get_from_socket(connfd);
    Client *t_c = clients_get(target_id);
    int client_id = clients_get_id_from_socket(connfd);

    int len = strlen(cmd);
    if(cmd[len-1] == '\n')  cmd[len-1] = '\0';

    // strip twice to remove first two words
    sprintf(send_buff, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", c->name, client_id+1, cmd, t_c->name, target_id+1);

    broadcast_all(send_buff);
}
