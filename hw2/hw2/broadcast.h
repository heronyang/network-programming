#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_user_connect(int shmid, struct sockaddr_in address);
void broadcast_user_disconnect(int shmid);
void broadcast_catch(int signo);
void broadcast_init(int connfd, int sm);

#endif
