#ifndef BROADCAST_H
#define BROADCAST_H

void broadcast_user_connect(struct sockaddr_in address);
void broadcast_user_disconnect();

void broadcast_cmd_name();
void broadcast_cmd_yell(char *buff);
void broadcast_cmd_tell(int target_id, char *buff);

void broadcast_catch(int signo);
void broadcast_init(int connfd);

/* tools */
int check_client_exist(int client_id);
int get_my_client_id();

#endif
