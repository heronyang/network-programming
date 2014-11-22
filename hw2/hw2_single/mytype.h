typedef struct {
    int valid;
    char *ip;
    uint16_t port;
    int socket;
    char name[NAME_SIZE];
} Client;
