#ifndef COMMON_H
#define COMMON_H

// Improtazione librerie

#include <ncurses.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

//Macro utilizzate

#define ALTEZZA_RANA 2
#define LARGHEZZA_RANA 3
#define VITE_INIZIALI 3
#define TEMPO_MANCHE 60
#define NUM_TANE 5
#define ALTEZZA_SPONDA 6
#define NUM_CORSIE 8
#define LARGHEZZA_MAX_COC (3 * LARGHEZZA_RANA)
#define LARGHEZZA_MIN_COC (2 * LARGHEZZA_RANA)
#define NUM_COC 4
#define RITARDO_GRANATA 40000
#define RITARDO_PROIETTILE 50000
#define PROB_PROIETTILE 150
#define MAX_OBJECTS 250
#define MAX_CHILD_PROCS (1 + 1 + (NUM_CORSIE * NUM_COC) + 1 + (NUM_CORSIE * NUM_COC) + 50)

//Strutture definite

enum ColorPairs {
    CP_FROG = 1, CP_GRASS, CP_SIDEWALK, CP_CROC, CP_TANA,
    CP_TANA_CHIUSA, CP_EYES,
    CP_INFO, CP_SCORE_BG, CP_FIELD_BG, CP_BULLET, CP_GRENADE
};

typedef enum {
    OBJ_FROG, OBJ_CROC, OBJ_TIMER_TICK, OBJ_INPUT_KEY,
    OBJ_BULLET, OBJ_GRENADE
} ObjectType;

typedef struct {
    int x, y;
} Posizione;

typedef struct {
    ObjectType tipo;
    int id;
    pid_t pid;
    Posizione pos;
    Posizione prev_pos;
    int larghezza;
    int altezza;
    int direzione;
    int corsia;
    int active;
    int owner_id;
    union {
        struct {
            bool riding_croc;
            int croc_id;
            int riding_offset_x;
        } stato_rana;
        struct {
            int key_code;
        } input;
    } data;
} GameObject;

typedef struct {
    int punteggio;
    int vite;
    int tempo_rimanente;
    int tane_occupate[NUM_TANE];
} StatoGioco;

typedef struct {
    int key;
    Posizione current_pos;
} CommandiRana;


extern StatoGioco stato;
extern Posizione posizioni_tane[NUM_TANE];
extern int max_x, max_y;
extern volatile bool game_over;
extern pid_t child_pids[MAX_CHILD_PROCS];
extern WINDOW *score, *campo, *info;

#endif 
