#define FIFO_PATH_LENGTH    100

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fd2;
char *fifo_path = "fifo_t";

int count = 0;

void fifo_setup() {

    if(mknod(fifo_path, S_IFIFO | 0666, 0) < 0) {
        perror("mknod");
    }

    /* write "Hi" to the FIFO */
    if((fd2 = open(fifo_path, O_NONBLOCK | O_RDONLY) ) < 0) {
        perror("open");
    }

}

void fifo_w() {

    int fd;
    char *content = (count % 2)?"hello" : "yoooo";

    if((fd = open(fifo_path, O_NONBLOCK | O_WRONLY) ) < 0) {
        perror("open");
    }
    if(write(fd, content, sizeof(content)) < 0)
        perror("write");
    close(fd);

    count ++;

}

void fifo_r() {

    char result[100];
    if(read(fd2, result, 100) < 0)
        perror("read");

    printf("result = %s\n", result);

}

void fifo_close() {

    close(fd2);
    unlink(fifo_path);

}
