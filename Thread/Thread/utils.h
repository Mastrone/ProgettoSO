#ifndef UTILS_H
#define UTILS_H

#include "common.h"

// Prototipi delle funzioni per la gestione del buffer + calcolo corsia
void init_buffer(GameObjectBuffer *buffer);
void produce(GameObjectBuffer *buffer, GameObject obj);
GameObject consume(GameObjectBuffer *buffer);
void cleanup_buffer(GameObjectBuffer *buffer);
int calc_corsia(int y, int max_y, int altezza_sponda, int num_corsie);

#endif
