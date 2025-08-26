#ifndef GAME_H
#define GAME_H

#include "common.h"

// Prototipi delle funzioni thread
void init_game(void);
void *input_thread(void *arg);
void *rana_thread(void *arg);
void *croc_thread(void *arg);
void *grenade_thread(void *arg);
void *projectile_thread(void *arg);
void *game_timer(void *arg);
void *consumer_thread(void *arg);

#endif
