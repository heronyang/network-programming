#ifndef CONSTANT_H
#define CONSTANT_H

#define DEBUG           0

#define PORT            33917

#define SIZE_SEND_BUFF  1000001
#define SIZE_READ_BUFF  1000001
#define SIZE_PIPE_BUFF  10001
#define MAX_PIPE_NUM    1001
#define BACKLOG         10

#define FALSE           0
#define TRUE            1

#define READ            0
#define WRITE           1

#define COMMAND_HANDLED -1

#define SKIP_SHIFT      2

#define SHM_KEY         5123
#define SHM_MSG_KEY     5124
#define SHM_NAME_KEY    5125
#define SHM_FIFO_LOCK_KEY   5126

#define CLIENT_MAX_NUM  30

#define MESSAGE_SIZE    10000
#define NAME_SIZE       21
#define TMP_STRING_SIZE 1200

#define IP_STRING_SIZE  20

#define FIFO_PATH_DIR   "tmp/"
#define PATH_LENGTH     1000

#endif
