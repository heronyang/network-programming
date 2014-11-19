#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_user_connect(struct sockaddr_in address);
void broadcast_user_disconnect();

void broadcast_cmd_name();

void broadcast_catch(int signo);
void broadcast_init(int connfd);

#endif
