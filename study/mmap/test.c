#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

    int fd = -1;
    //const char str[] = "heron yang";
    //const char new_str[] = "ducky";

    pid_t pid;
    pid = fork();

    int *anon = (int *)mmap(NULL, sizeof(int *), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);       //

    if (anon == MAP_FAILED)                                           //
        errx(1, "either mmap");

    //strcpy(anon, str);
    *anon = 12;

    if(pid < 0) {                       // if error

        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);

    } else if (pid == 0) {              // if child

        //strcpy(anon, new_str);
        *anon = 13;
        printf("child: %d\n", *anon);
        munmap(anon, sizeof(anon));

        exit(0);

    } else {                            // if parent

        wait();
        printf("parent: %d\n", *anon);
        munmap(anon, sizeof(anon));

    }

}
