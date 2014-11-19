#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variable.h"
#include "constant.h"

int g_shmid_name;

void setname(int client_id, char *name) {
    char *s_name;
    if ((s_name = shmat(g_shmid_name, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    strcpy((s_name + client_id * NAME_SIZE), name);
    shmdt(s_name);
}

char *getname(int client_id) {
    char *s = malloc(sizeof(char) * NAME_SIZE);
    char *s_name;
    if ((s_name = shmat(g_shmid_name, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    strcpy(s, (s_name + client_id * NAME_SIZE));
    shmdt(s_name);
    return s;
}
