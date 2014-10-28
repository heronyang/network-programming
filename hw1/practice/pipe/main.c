#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

int main (){

    int fd[2], nbytes;
    pid_t childpid;
    char readbuffer[10001];
    char *string = "hello!!";

    pipe(fd);

    if((childpid = fork()) == -1){
        perror("fork");
        exit(1);
    }

    if(childpid == 0) {

        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);

        //printf("test!!\n");             // try to on/off this line
        write(fd[1], string, strlen(string));   // try to on/off this line
        execlp("ls","ls", "-l", (char *)NULL);  // try to on/off this line

        exit(0);

    } else {

        close(fd[1]);
        wait(NULL);

        int i;

        memset(readbuffer, 0, sizeof(readbuffer));
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("\n\n*received string: %s\n", readbuffer);
        for( i=0 ; i<100 ; i++ ) {
            printf("%c(%3d) ", readbuffer[i], readbuffer[i]);
            if( (i%10) == 9 ) printf("\n");
        }
        printf("\n");

        memset(readbuffer, 0, sizeof(readbuffer));
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("\n\n*received string: %s\n", readbuffer);
        for( i=0 ; i<100 ; i++ ) {
            printf("%c(%3d) ", readbuffer[i], readbuffer[i]);
            if( i%10 == 9 ) printf("\n");
        }
        printf("\n");

        memset(readbuffer, 0, sizeof(readbuffer));
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("\n\n*received string: %s\n", readbuffer);
        for( i=0 ; i<100 ; i++ ) {
            printf("%c(%3d) ", readbuffer[i], readbuffer[i]);
            if( i%10 == 9 ) printf("\n");
        }
        printf("\n");

    }

    return 0;
}
