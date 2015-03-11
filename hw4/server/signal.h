#ifndef SIGNAL_H
#define SIGNAL_H

void signal_init();
void catch_chld(int snum);
void catch_int(int i);

#endif
