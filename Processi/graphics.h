#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"

// Prototipi delle funzioni di disegno
void draw_bordi(void);
void draw_tane(void);
void draw_rana(GameObject *frog);
void draw_coc(GameObject *croc);
void draw_proiettile(GameObject *bullet);
void draw_granata(GameObject *grenade);
bool draw_fine(bool vittoria, bool quit);

#endif 
