#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    int valid;
    int pid;
    char *name;
    char *ip;
    uint16_t port;
} Client;

int main()
{
    char c;
    int shmid;
    key_t key;
    Client *shm, *s;

    key = 5678;

    int size = sizeof(Client) * 10;
    printf("size = %d\n", size);
    if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }


    pid_t pid;
    pid = fork();

    if(pid < 0) {                       // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid == 0) {              // if child

        if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
            perror("shmat");
            exit(1);
        }

        shm[4].pid = 100;


        shmdt(shm);

        exit(0);

    } else {                            // if parent

        wait();

        if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
            perror("shmat");
            exit(1);
        }

        printf("pid = %d\n", shm[4].pid);

        shmdt(shm);

    }

    // remove allocated shm in OS
    shmctl(shmid, IPC_RMID, NULL);

    //
    exit(0);
}
