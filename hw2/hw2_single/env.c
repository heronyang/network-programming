#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constant.h"
// NOTICE: now is only snapshoting for PATH variable

char clients_env_var[CLIENT_MAX_NUM][PATH_LENGTH];
char *var_key = "PATH";
char *default_value = "bin:.";

/* per server process */
void env_init() {

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        strcpy(clients_env_var[i], default_value);
    }

    chdir("./ras");

}

/* per client connection */
void env_set(int client_id) {

    setenv(var_key, clients_env_var[client_id], TRUE);

}

void env_save(int client_id) {

    char *r;
    r = getenv(var_key);
    strcpy( clients_env_var[client_id], r );

}

/* */
void env_clean(int client_id) {

    strcpy(clients_env_var[client_id], default_value);

}

