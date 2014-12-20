/*
 * Note:
 *      STDERR: for debug
 *      STDOUT: dup to socket
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#define PORT            33920
#define BACKLOG         10
#define BUF_SIZE        10240

#define EXT_CGI         1
#define EXT_HTML        2
#define EXT_TEXT        3

#define DOCUMENT_ROOT   "../www/"

/* Globals */
int child_count = 0;

/* 
 * Signal Handlers 
 */
void catch_child(int snum) {

    int pid;
    int status;

    pid = wait(&status);
    fprintf(stderr, "parent: child process pid=%d exited with value %d\n", pid, WEXITSTATUS(status));

    if(pid != -1)   child_count --;

}

void catch_int(int i) {

    if(child_count!=0) {
        fprintf(stderr, "unfinished client, can't close\n");
        return;
    }

    exit(0);    // end program

}

/*
 * Sock
 */
void parse_request(char *buf, char *res) {
    char *r;
    strtok(buf, " ");
    r = strtok(NULL, " ");
    strncpy(res, r+1, strlen(r)-1);
}

void parse_qs(char *req, char *filename) {
    char *r, *qs;

    r = strtok(req, "?");
    if(r == NULL)   return;
    strcpy(filename, r);
    fprintf(stderr, "filename= %s\n", filename);

    qs = strtok(NULL, " ");
    if(qs == NULL)   return;
    fprintf(stderr, "QS= %s\n", qs);
    setenv("QUERY_STRING", qs, 1);
}

int get_ext(char *filename) {

    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)         return EXT_TEXT;
    if( strcmp(dot + 1, "cgi") == 0 )   return EXT_CGI;
    if( strcmp(dot + 1, "html") == 0 )  return EXT_HTML;

    return EXT_TEXT;

}

void write_header(int sock, int ext) {
    char *content_html = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    char *content_text = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    if(ext == EXT_HTML) write(sock, content_html, strlen(content_html));
    else                write(sock, content_text, strlen(content_text));
}

void write_short_header(int sock) {
    char *content = "HTTP/1.1 200 OK\n";
    write(sock, content, strlen(content));
}

void request_handler(int sock) {

    // read: parse GET
    char buf[BUF_SIZE], req[BUF_SIZE], filename[BUF_SIZE], filepath[BUF_SIZE];
    int fsize, ext;
    FILE *fp;

    read(sock, buf, sizeof(buf));
    fprintf(stderr, "read: %s\n", buf);

    parse_request(buf, req);
    fprintf(stderr, "req: %s\n", req);

    parse_qs(req, filename);

    strcpy(filepath, DOCUMENT_ROOT);
    strcat(filepath, filename);

    ext = get_ext(filename);

    if(ext == EXT_CGI) {

        // handle cgi
        fprintf(stderr, "is cgi\n");
        write_short_header(sock);

        dup2(sock, STDIN_FILENO);
        dup2(sock, STDOUT_FILENO);
        
        if( execl(filepath, filepath, NULL) <0 ) {
            perror("execl");
        }

    } else {

        // normal files
        fp = fopen(filepath, "r");
        if( fp == NULL ) {
            perror("fopen");
        }

        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        memset(buf, 0, BUF_SIZE);
        if( fsize != fread(buf, fsize, 1, fp) ) {
            perror("fread");
        }

        fprintf(stderr, "file: %s\n", buf);

        write_header(sock, ext);
        write(sock, buf, strlen(buf));

        fclose(fp);

    }

    // if normal files: fread -> write socket
    // if .cig -> dup2 -> exec

}

int client_handler(int sock) {

    pid_t pid;
    pid = fork();

    if(pid < 0) {

        fprintf(stderr, "Fork failed\n");
        exit(1);

    } else if (pid == 0) {

        request_handler(sock);
        exit(0);

    } else {

        int return_val;    
        waitpid(pid, &return_val, 0);
        if(WEXITSTATUS(return_val) == 1)  return 1;

    }

    return 0;


}

/*
 * Main
 */
int main(int argc, char *argv[]) {

    /* variables */
    int listenfd = 0, addrlen;
    struct sockaddr_in serv_addr; 

    /* init */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    /* setup */
    serv_addr.sin_family = AF_INET;                 // internet protocols (IPv4)
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // address
    serv_addr.sin_port = htons(PORT);               // port

    /* socket - bind */
    int optval = 1;
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1 ) {
        perror("setsockopt");
    }
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    /* socket - listen */
    listen(listenfd, BACKLOG); 

    addrlen = sizeof(serv_addr);

    fprintf(stderr, "start listen on port %d...\n", PORT);
    while(1) {

        /* socket - accept */
        int csock = accept(listenfd, (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen); 
        child_count ++;

        /* fork to handle */
        pid_t pid;
        pid = fork();

        /* detect child termination */
        signal(SIGCHLD, catch_child);

        if(pid<0) {                     // if error

            fprintf(stderr, "Fork failed\n");
            exit(EXIT_FAILURE);

        } else if (pid ==0) {           // if child

            // client: init
            fprintf(stderr, "accepted connection: %d\n", csock);

            // client: handle
            client_handler(csock);

            // client: close
            if(close(csock) < 0) {
                perror("close");
            }

            exit(0);

        } else {

            if(close(csock) < 0) {
                perror("close");
            }
            fprintf(stderr, "closed connection: %d\n", csock);

        }

    }

    return 0;
    //
}
