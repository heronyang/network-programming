#ifndef TYPE_H
#define TYPE_H

typedef struct {
    char *ip;
    char *port;
    char *file;
    int socket;
    FILE *fp;
} Request;

#endif
