#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_new_comer(struct sockaddr_in serv_addr);

void broadcast_sender_all(int shmid);
void broadcast_sender_pid(int pid);

#endif
