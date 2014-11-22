#ifndef ENV_H
#define ENV_H

void env_init();
void env_set(int connfd);
void env_save(int connfd);
void env_clean(int confd);

#endif
