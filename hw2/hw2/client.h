#ifndef CLIENT_H
#define CLIENT_H

char **command_decode(char *command);
void socket_error_message(int connfd);
void setenv_helper(int connfd);
void printenv_helper(int connfd);
int read_helper(int connfd, char *buf);
int prompt(int connfd, int shmid);
void welcome_msg(int connfd);
void debug_print_command(char** argv_s, int p_n);
char **extract_command(int len);
void init_env();
void command_handler(int connfd);
void client_handler(int connfd, int shmid);

#endif
