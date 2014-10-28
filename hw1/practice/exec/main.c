#include <stdio.h>

/* This program forks and and the prints whether the process is
 *   - the child (the return value of fork() is 0), or
 *   - the parent (the return value of fork() is not zero)
 *
 * When this was run 100 times on the computer the author is
 * on, only twice did the parent process execute before the
 * child process executed.
 *
 * Note, if you juxtapose two strings, the compiler automatically
 * concatenates the two, e.g., "Hello " "world!"
 */

int main( void ) {
    char *argv[3] = {"Command-line", ".", NULL};

    int pid = fork();

    if ( pid == 0 ) {
        execvp( "find", argv );
    }

    /* Put the parent to sleep for 2 seconds--let the child finished executing */
    wait( 2 );

    printf( "Finished executing the parent process\n"
            " - the child won't get here--you will only see this once\n" );

    return 0;
}
