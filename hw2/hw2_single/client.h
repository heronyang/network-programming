#ifndef CLIENT_H
#define CLIENT_H

void welcome_msg(int connfd);
void init_env();
void command_handler(int connfd);
int client_handler(int connfd);
void print_prompt_sign(int connfd);

#endif
