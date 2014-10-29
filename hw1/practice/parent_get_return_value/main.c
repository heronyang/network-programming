#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main (void) {

    pid_t pid;
    pid = fork ();

    if (pid < (pid_t) 0) {
        /* The fork failed. */
        fprintf (stderr, "Fork failed.\n");
        return EXIT_FAILURE;
    }
    else if (pid == (pid_t) 0) {             // child
        return -1;
    }
    else {                                   // parent
        int returnStatus;    
        waitpid(pid, &returnStatus, 0);  // Parent process waits here for child to terminate.
        printf("return status = %d\n", WEXITSTATUS(returnStatus));

        return EXIT_SUCCESS;
    }
}
