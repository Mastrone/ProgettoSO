#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"

// Prototipi delle funzioni di disegno
void draw_sponde(void);
void draw_tane(void);
void draw_rana(GameObject *frog);
void draw_coc(GameObject *croc);
void draw_granata(GameObject *grenade);
void draw_proiettile(GameObject *projectile);
bool draw_fine(bool vittoria, bool quit);

#endif