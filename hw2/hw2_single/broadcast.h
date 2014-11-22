#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_user_connect(int connfd, struct sockaddr_in address);
void broadcast_user_disconnect(int connfd);

void broadcast_cmd_name(int connfd);
void broadcast_cmd_yell(int connfd, char *buff);
void broadcast_cmd_tell(int connfd, int target_id, char *buff);
/*

void broadcast_cmd_fifo_in(int source_id, char *cmd);
void broadcast_cmd_fifo_out(int target_id, char *cmd);
*/

/* tools */
/*
int check_client_exist(int client_id);
int get_my_client_id();
*/

#endif
