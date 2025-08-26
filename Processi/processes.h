#ifndef PROCESSES_H
#define PROCESSES_H

#include "common.h"

// Prototipi delle funzioni per i processi
void input_process(int pipe_write_fd);
void rana_process(int pipe_write_fd, int pipe_read_update_fd);
void coc_process(int pipe_write_fd, int id, int start_x, int y, int larghezza, int direzione, int corsia, int loop_distance, int speed);
void proettile_process(int pipe_write_fd, int id, int owner_id, Posizione start_pos, int direzione);
void granata_process(int pipe_write_fd, int id, int owner_id, Posizione start_pos, int direzione);
void timer_process(int pipe_write_fd);

#endif 
