#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void sig_handler(int signo)
{
    if (signo == SIGUSR1)
        printf("[pid=%d] received SIGUSR1\n", (int)getpid() );
    else
        printf("received unknown signal\n");
}

int main(void)
{

    pid_t pid;
    pid = fork();


    if(pid<0) {                     // if error

        printf("Fork failed\n");
        exit(1);

    } else if (pid ==0) {           // if child

        printf("child: pid = %d\n", (int) getpid ());

        sleep(2);
        printf("child send signal\n");
        kill(getppid(), SIGUSR1);
        exit(1);

    } else {                        // if parent

        if (signal(SIGUSR1, sig_handler) == SIG_ERR)
            printf("\ncan't catch SIGUSR1\n");

        printf("parent: pid = %d\n", (int) getpid ());

        while(1) 
            sleep(1);

    }

    return 0;
}
