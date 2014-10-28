#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

char string[] = "Hello, world\n";
char readbuffer[80];

int main (){

    pid_t pid;
    int mypipe[2], nbytes;

    if(pipe(mypipe)) {
        printf("pipe create error\n");
        exit(-1);
    }

    pid = fork();
    if(pid < (pid_t)0){
        perror("fork");
        exit(1);
    }

    else if(pid == (pid_t)0) {
        if(close(mypipe[0]) < 0) {
            printf("[child] close error!\n");
            exit(-1);
        }
        write(mypipe[1], string, (strlen(string)+1));
        exit(0);
    }
    else {
        wait(NULL);
        if(close(mypipe[1]) < 0) {
            printf("[parent] close error!\n");
            exit(-1);
        }
        nbytes = read(mypipe[0], readbuffer, sizeof(readbuffer));
        printf("received string: %s", readbuffer);
    }

    return 0;
}
