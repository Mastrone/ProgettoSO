#ifndef UTILS_H
#define UTILS_H

#include "common.h"

//Prototipi funzioni

void handle_sigint_parent();
void handle_sigterm_child();
void cleanup_processes(void);
void init_game(void);
int calc_corsia(int y, int max_y, int shore_altezza, int num_corsie);
ssize_t read_full(int fd, void *buf, size_t count);

#endif 
