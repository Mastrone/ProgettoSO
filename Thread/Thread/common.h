#ifndef COMMON_H
#define COMMON_H

// Importazione delle librerie necessarie
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// Definizione delle costanti del gioco
#define ALTEZZA_RANA 2
#define LARGHEZZA_RANA 3
#define DIMENSIONE_BUFFER 256
#define VITE_INIZIALI 3
#define TEMPO_MANCHE 60
#define NUM_TANE 5
#define ALTEZZA_SPONDA 6
#define NUM_CORSIE 8
#define LARGHEZZA_COC_MIN (2 * LARGHEZZA_RANA)
#define LARGHEZZA_COC_MAX (3 * LARGHEZZA_RANA)
#define NUM_COC 4
#define NUM_MAX_OGGETTI 200
#define RITARDO_GRANATA 50000
#define RITARDO_PROIETTILE 50000
#define PROB_PROIETTILE 250
#define DISTANZA_MIN_COC (LARGHEZZA_RANA * 4)
#define DISTANZA_MAX_COC (LARGHEZZA_RANA * 8)

// Definizione delle strutture dati

typedef struct {
    int x, y;
} Posizione;

enum ColorPairs {
    CP_RANA = 1,
    CP_ERBA,
    CP_PAVIMENTO,
    CP_COC,
    CP_TANA,
    CP_TANA_CHIUSA,
    CP_EYES,
    CP_INFO,
    CP_SCORE_BG,
    CP_FIELD_BG,
    CP_GRENADE,
    CP_PROJECTILE
};

typedef enum {
    OBJ_RANA,
    OBJ_COC,
    OBJ_GRANATA,
    OBJ_PROIETTILE
} ObjectType;

typedef struct {
    ObjectType tipo;
    int id;
    Posizione pos;
    Posizione prev_pos;
    int larghezza;
    int altezza;
    int direzione;
    int corsia;
    int active;
    bool riding_croc;
    int croc_id;
} GameObject;

typedef struct {
    GameObject buffer[DIMENSIONE_BUFFER];
    int in;
    int out;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
} GameObjectBuffer;

typedef struct {
    int punteggio;
    int vite;
    int tempo_rimanente;
    int tane_occupate[NUM_TANE];
} StatoGioco;

typedef struct {
    int corsia;
    int start_x;
    int direzione;
    int velocita;
    int larghezza;
    int distanza_loop;
    int id;
} CocArgs;

typedef struct {
    Posizione start_pos;
    int direzione;
} ProiettileArgs;


extern GameObjectBuffer buffer;
extern pthread_t coc_threads[NUM_CORSIE];
extern StatoGioco stato;
extern Posizione posizioni_tane[NUM_TANE];
extern int max_x, max_y;
extern volatile bool game_over;
extern volatile int input_key;
extern volatile bool user_quit;
extern volatile bool granata_sparata;
extern pthread_mutex_t input_mutex;

extern volatile bool manche_terminata;
extern volatile bool livello_vinto;

extern pthread_mutex_t mutex_pos_rana;
extern Posizione pos_r;
extern bool pos_r_update;

extern pthread_mutex_t dynamic_object_id_mutex;
extern int next_dynamic_object_id;

extern WINDOW *score, *campo, *info;


#endif
