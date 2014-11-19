#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "constant.h"
#include "pipe.h"
#include "fork_exec.h"
#include "client.h"
#include "mytype.h"

/*
 * Globals
 */
char send_buff[SIZE_SEND_BUFF];
char read_buff[SIZE_READ_BUFF];

char **argv;
int argc = 0;

/*
 * Shell
 */
char **command_decode(char *command) {

    char *token = " \t\n\r";
    char *p = strtok(command, token);

    argc = 0;
    while(p) {
        argc ++;
        argv = realloc(argv, sizeof(char *) * argc);

        if(argv == NULL)    exit(EXIT_FAILURE);

        argv[argc-1] = p;
        p = strtok(NULL, token);
    }

    // for the last extra one
    argv = realloc(argv, sizeof(char *) * (argc+1));
    argv[argc] = NULL;

    return argv;

}

void socket_error_message(int connfd) {
    memset(send_buff, 0, sizeof(send_buff)); 
    snprintf(send_buff, sizeof(send_buff), "Invalid Inputs.\n");
    write(connfd, send_buff, strlen(send_buff)); 
}

void setenv_helper(int connfd) {
    if(argc != 3) {
        socket_error_message(connfd);
        return;
    }
    setenv(argv[1], argv[2], TRUE);
}

void printenv_helper(int connfd) {

    if(argc != 2) {
        socket_error_message(connfd);
        return;
    }

    char *r;
    r = getenv(argv[1]);

    memset(send_buff, 0, sizeof(send_buff)); 
    if(r)   snprintf(send_buff, sizeof(send_buff), "%s=%s\n", argv[1], r);
    else    snprintf(send_buff, sizeof(send_buff), "Variable Not Found.\n");
    write(connfd, send_buff, strlen(send_buff)); 

}

int read_helper(int connfd, char *buf) {
    int count = 0;
    char c;
    while(1) {
        if(!read(connfd, &c, 1))  break;
        buf[count++] = c;
        if(c == '\n')   break;
    }
    buf[count] = '\0';
    return count;
}

void cmd_who(int connfd, int shmid) {

    char content[SIZE_SEND_BUFF] = "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n";
    char t_s[TMP_STRING_SIZE];

    int pid = getpid(), i;

    Client *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (Client *) -1) {
        perror("shmat");
        exit(1);
    }
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {

        if(!shm[i].valid)   continue;

        sprintf(t_s, "%d\t%s\t%s/%d", i, shm[i].name, shm[i].ip, shm[i].port);

        if(shm[i].pid == pid) {
            strcat(t_s, "\t<- me\n");
        } else {
            strcat(t_s, "\n");
        }

        fprintf(stderr, "valid found: %d, :%s\n", i, t_s);

        strcat(content, t_s);

    }
    shmdt(shm);

    snprintf(send_buff, sizeof(send_buff), content);
    write(connfd, send_buff, strlen(send_buff)); 

}

int prompt(int connfd, int shmid) {

    int r = 0;

    memset(send_buff, 0, SIZE_SEND_BUFF); 
    memset(read_buff, 0, SIZE_READ_BUFF); 

    snprintf(send_buff, SIZE_SEND_BUFF, "%% ");
    write(connfd, send_buff, strlen(send_buff)); 
    // r = read(connfd, read_buff, SIZE_READ_BUFF);
    r = read_helper(connfd, read_buff);
    if(r == 1)  return COMMAND_HANDLED;

    argv = command_decode(read_buff);
    if(strcmp(argv[0], "exit") == 0)  return 0;   // same as end
    if(strcmp(argv[0], "setenv") == 0) {
        setenv_helper(connfd);
        return COMMAND_HANDLED;
    }
    if(strcmp(argv[0], "printenv") == 0) {
        printenv_helper(connfd);
        return COMMAND_HANDLED;
    }
    if(strcmp(argv[0], "who") == 0) {
        cmd_who(connfd, shmid);
        return COMMAND_HANDLED;
    }

    return r;

}

void welcome_msg(int connfd) {

    char *msg = "****************************************\n\
** Welcome to the information server. **\n\
****************************************\n";
    snprintf(send_buff, sizeof(send_buff), msg);
    write(connfd, send_buff, strlen(send_buff)); 

}

void debug_print_command(char** argv_s, int p_n) {
    int k;
    fprintf(stderr, "\n==========\n(pipe)exec: ");
    fprintf(stderr, "p_n = %d\n", p_n);
    for( k=0 ; argv_s[k]!=NULL ; k++ ){
        fprintf(stderr, ".%s", argv_s[k]);
    }
    fprintf(stderr, "\n");
}

/*
 * Handler (command, client)
 */
// helper tool
char **extract_command(int len) {

    int j;
    char **argv_s = malloc(sizeof(char *) * (len));

    for( j=0 ; j<argc ; j++ ) {
        // move to sub argv, which will be exec
        if( j<len ) {
            argv_s[j] = malloc(sizeof(char) * sizeof(argv[j]));
            strcpy(argv_s[j], argv[j]);
        }

        // shift argv
        if( j<(argc-len-1) ) {
            argv[j] = malloc(sizeof(char) * sizeof(argv[j+len+1]));
            strcpy(argv[j], argv[j+len+1]);
        }
    }

    argc -= (len+1);
    argv[argc] = NULL;
    argv_s[len] = NULL;

    return argv_s;

}

// a whole line as input (pipe may be included)
void init_env() {

    // change to working directory
    chdir("./ras");

    // only support custom bin/
    setenv("PATH", "bin:.", TRUE);

}
void command_handler(int connfd) {

    int i;
    int is_pipe;

    while(1) {

        is_pipe = FALSE;

        for( i=0 ; i<argc ; i++ ) {
            if(argv[i] && argv[i][0] == '|') {

                int p_n = 1;
                if(strlen(argv[i]) == 1)  p_n = 1;
                else    sscanf(argv[i], "|%d", &p_n);

                is_pipe = TRUE;

                //
                int r;
                char **argv_s = extract_command(i);

                if(DEBUG)   debug_print_command(argv_s, p_n);

                if( (r=fork_and_exec_pipe(connfd, argv_s, p_n)) == EXIT_FAILURE) {
                    return;
                } else if (r != SKIP_SHIFT) {
                    pipe_shift();
                }

                break;

            }
            if(argv[i] && argv[i][0] == '>') {

                char *filepath = argv[i+1];
                char **argv_s = extract_command(i);
                if(!filepath)   fprintf(stderr, "filepath error\n");
                if( fork_and_exec_file(connfd, argv_s, filepath) == EXIT_FAILURE ) {
                    return;
                }
                return;

            }
        }

        if(!is_pipe)    break;

    }

    if(argc == 0)   return;
    fork_and_exec_last(connfd, argv);
    pipe_shift();

}

// handle one socket connection
void client_handler(int connfd, int shmid) {

    init_env();

    // handle (first)
    welcome_msg(connfd);

    // handle (rest)
    while(1) {
        int r = prompt(connfd, shmid);
        if(r == COMMAND_HANDLED) continue;
        if(!r)  break;
        command_handler(connfd);
    }

}

