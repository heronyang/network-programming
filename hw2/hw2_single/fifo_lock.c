#include <stdio.h>
#include <stdlib.h>

#include "constant.h"
#include "variable.h"

char fifo_lock[CLIENT_MAX_NUM][CLIENT_MAX_NUM];

void fifo_lock_init() {

    int i;
    for( i=0 ; i<CLIENT_MAX_NUM ; i++ ) {
        for( j=0 ; j<CLIENT_MAX_NUM ; j++ ) {
            fifo_lock[i][j] = 0;
        }
    }

}

void fifo_lock_close() {
    // ;
}

void fifo_lock_set(int source_id, int target_id, char val) {
    fifo_lock[source_id][target_id] = val;
}

char fifo_lock_get(int source_id, int target_id) {
    return fifo_lock[source_id][target_id];
}
