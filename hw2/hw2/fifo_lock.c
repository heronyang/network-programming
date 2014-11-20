#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#include "constant.h"

int g_shmid_fifo_lock;

void fifo_lock_init() {

    // allocation
    key_t key = SHM_FIFO_LOCK_KEY;
    int shm_size = sizeof(char) * CLIENT_MAX_NUM * CLIENT_MAX_NUM;
    if ((g_shmid_fifo_lock = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // init value
    char *fifo_lock;
    if ((fifo_lock = shmat(g_shmid_fifo_lock, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    
    int i;
    for( i=0 ; i<CLIENT_MAX_NUM * CLIENT_MAX_NUM ; i++ )    fifo_lock[i] = 0;

    shmdt(fifo_lock);

}

void fifo_lock_close() {
    shmctl(g_shmid_fifo_lock, IPC_RMID, NULL);
}

void fifo_lock_set(int source_id, int target_id, char val) {

    char *fifo_lock;
    if ((fifo_lock = shmat(g_shmid_fifo_lock, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    * (fifo_lock + source_id * CLIENT_MAX_NUM + target_id) = val;
    shmdt(fifo_lock);

}

char fifo_lock_get(int source_id, int target_id) {

    char *fifo_lock;
    if ((fifo_lock = shmat(g_shmid_fifo_lock, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    char r = * (fifo_lock + source_id * CLIENT_MAX_NUM + target_id);
    shmdt(fifo_lock);

    return r;
}
