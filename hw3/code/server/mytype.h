#ifndef MYTYPE_H
#define MYTYPE_H

#include <stdint.h>

typedef struct {
    int valid;
    char *ip;
    uint16_t port;
    int socket;
    char name[NAME_SIZE];
} Client;

#endif
