#ifndef VARIABLE_H
#define VARIABLE_H

#include "constant.h"
#include "mytype.h"

extern Client clients[CLIENT_MAX_NUM];
extern int client_count;

extern int fifo_fd[CLIENT_MAX_NUM][CLIENT_MAX_NUM];

#endif
