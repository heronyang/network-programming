#ifndef FIFO_LOCK_H
#define FIFO_LOCK_H

void fifo_lock_init();
void fifo_lock_close();
void fifo_lock_set(int source_id, int target_id, char val);
char fifo_lock_get(int source_id, int target_id);

#endif
