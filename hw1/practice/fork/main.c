/*
 * Date:    10/20/2013
 * Author:  Heron Yang
 * Name:    Simple Shell
 *
 * This is homework 1st for "Introduction to Operating System", Simple Shell.
 *
 * Content: This program simply ask the user to insert shell commands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/* Global Variables */
const int CMD_STR_SIZE = 1000;
char *Cmd_str;

/* Function Declarations */
void prompt();
void run_command();

//
int main() {

    // allocate cmd_str
    Cmd_str = (char*)malloc(CMD_STR_SIZE);

    // prompt for the first time
    prompt();

    while(1) {

        // fork here **
        pid_t pid;
        pid = fork();

        if(pid <0) {
            // if error, fork failed
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if(pid == 0){
            // if child
            run_command();
            exit(0);
        }
        else {
            // if parent
            wait(NULL);
            prompt();
        }
    }

    return 0;
}

/*
 * Prints out prompt character and read user's input command.
 * Avoid using gets because of the security issue.
 */
void prompt() {
    printf(">");
    fgets(Cmd_str, CMD_STR_SIZE, stdin);
}

/*
 * Run a single command from the user's input,
 * split the command into argv string array first,
 * then call execvp to execute.
 */
void run_command() {

    // split the original string
    char **argv     = NULL;
    char  *token    = " \t\n\r";                // characters for splitting
    char  *p        = strtok(Cmd_str, token);
    int argc = 0, i;

    while(p) {
        argc ++;
        argv = realloc(argv, sizeof(char *) * argc);

        // if memory allocation failed
        if(argv == NULL)    exit(-1);

        argv[argc-1] = p;
        p = strtok(NULL, token);
    }

    // for the last extra one
    argv = realloc(argv, sizeof(char *) * (argc+1));
    argv[argc] = 0;     // set last one as NULL

    // exexcute here
    execvp(argv[0], argv);

    // error handler
    fprintf(stderr, "Failed to execvp() '%s' (%d: %s)\n", argv[0], errno, strerror(errno));

    // free the memory
    free(argv);
}

//
