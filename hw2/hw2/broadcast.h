#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_new_comer(int shmid, struct sockaddr_in address);
void broadcast_catch(int signo);
void broadcast_init(int connfd, int sm);

#endif
