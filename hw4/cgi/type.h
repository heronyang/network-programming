#ifndef TYPE_H
#define TYPE_H

typedef struct {
    char *ip;
    char *port;
    char *file;
    char *sh;
    char *sp;
    int socket;
    FILE *fp;
} Request;

#endif
