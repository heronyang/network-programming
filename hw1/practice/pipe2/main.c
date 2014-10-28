#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#define IN  0
#define OUT 1

int main (){

    int fd[2], fd2[2], nbytes;
    pid_t childpid, childpid2;
    char string[] = "Hello, world\n";
    char readbuffer[80], readbuffer2[80];

    pipe(fd);

    if((childpid = fork()) == -1){
        perror("fork");
        exit(1);
    }


    if(childpid == 0) {

        close(fd[IN]);

        pipe(fd2);

        if((childpid2 = fork()) == -1){
            perror("fork");
            exit(1);
        }
        if(childpid2 == 0) {
            write(fd2[OUT], string, (strlen(string)+1));
            exit(0);
        } else {
            wait(NULL);
            nbytes = read(fd2[IN], readbuffer2, sizeof(readbuffer2));
            printf("(inner) received string: %s", readbuffer2);
            write(fd[OUT], readbuffer2, (strlen(readbuffer2)+1));
            write(fd[OUT], string, (strlen(string)+1));
            exit(0);
        }

    } else {
        close(fd[OUT]);
        nbytes = read(fd[IN], readbuffer, sizeof(readbuffer));
        printf("received string: %s", readbuffer);
    }

    return 0;
}
