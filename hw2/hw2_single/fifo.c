#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>    //close
#include <string.h>

#include "constant.h"
#include "variable.h"
#include "broadcast.h"
#include "fifo_lock.h"

int fifo_fd[CLIENT_MAX_NUM][CLIENT_MAX_NUM];

void fifo_init() {

    int i, j;
    char fifo_path[PATH_LENGTH];

    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        for( j=0 ; j<CLIENT_MAX_NUM ; j++ ) {

            sprintf(fifo_path, "%sclient_%d_%d", FIFO_PATH_DIR, i, j);

            if(mknod(fifo_path, S_IFIFO | 0666, 0) < 0) {
                perror("mknod");
            }
            if((fifo_fd[i][j] = open(fifo_path, O_NONBLOCK | O_RDONLY) ) < 0) {
                perror("open(RO)");
            }

            if(DEBUG)   fprintf(stderr, "open: fifo_path = %s\n", fifo_path);

        }

    }
}

void fifo_close(int client_id) {

    int i;

    // clean up unread fifo
    char c;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        if(fifo_lock_get(i, client_id)) {
            while(read(fifo_fd[i][client_id], &c, 1) != 0) {
                ;
            }
            fifo_lock_set(i, client_id, FALSE);
        }
    }

}

void fifo_finalize() {

    int i, j;
    char fifo_path[PATH_LENGTH];

    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        for( j=0 ; j<CLIENT_MAX_NUM ; j++ ) {

            if(close(fifo_fd[i][j]) < 0) {
                perror("close (fifo)");
            }

            sprintf(fifo_path, "%sclient_%d_%d", FIFO_PATH_DIR, i, j);
            if(unlink(fifo_path) < 0) {
                perror("unlink");
            }

        }

    }

}
