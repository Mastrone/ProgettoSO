
#include "common.h"
#include "graphics.h"
#include "processes.h"
#include "utils.h"
#include <unistd.h>
#include <signal.h>

// Definizione delle variabili globali
StatoGioco stato;
Posizione posizioni_tane[NUM_TANE];
int max_x, max_y;
volatile bool game_over = 0; // Flag per la terminazione globale
pid_t child_pids[MAX_CHILD_PROCS] = {0}; // Array per memorizzare i PID dei processi figli
WINDOW *score, *campo, *info;


int main() {
    // Inizializzazione del gioco e di ncurses
    system("echo -ne '\e[8;33;80t'"); // Forza la dimensione del terminale
    usleep(100000); // Pausa per dare tempo al terminale di ridimensionarsi
    signal(SIGINT, handle_sigint_parent); // Gestisce Ctrl+C
    initscr(); 
    noecho(); 
    curs_set(0); 
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); 
    start_color();
    
    // Inizializzazione delle coppie di colori
    init_pair(CP_FROG, COLOR_WHITE, COLOR_GREEN); 
    init_pair(CP_TANA_CHIUSA, COLOR_BLACK, COLOR_WHITE);
    init_pair(CP_GRASS, COLOR_YELLOW, COLOR_GREEN); 
    init_pair(CP_SIDEWALK, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CP_EYES, COLOR_RED, COLOR_GREEN); 
    init_pair(CP_SCORE_BG, COLOR_WHITE, COLOR_BLACK);
    init_pair(CP_INFO, COLOR_WHITE, COLOR_BLACK);
    init_pair(CP_FIELD_BG, COLOR_WHITE, COLOR_BLUE);
    init_pair(CP_TANA, COLOR_WHITE, COLOR_GREEN); 
    init_pair(CP_CROC, COLOR_BLACK, COLOR_GREEN);
    init_pair(CP_BULLET, COLOR_RED, COLOR_BLUE); 
    init_pair(CP_GRENADE, COLOR_WHITE, COLOR_RED);

    // Creazione e configurazione delle finestre di gioco
    score = newwin(2, COLS, 0, 0); campo = newwin(28, COLS, 2, 0); 
    info = newwin(2, COLS, 30, 0);
    wbkgd(score, COLOR_PAIR(CP_SCORE_BG)); 
    wbkgd(info, COLOR_PAIR(CP_INFO)); 
    wbkgd(campo, COLOR_PAIR(CP_FIELD_BG));
    refresh(); getmaxyx(campo, max_y, max_x);

    bool play_again = false;
    // Ciclo do-while per permettere di rigiocare
    do {
        init_game();

        // Ciclo while per la partita, continua finché ci sono vite
        while (stato.vite > 0) {
            bool manche_terminata = false;
            memset(child_pids, 0, sizeof(child_pids)); // Azzera l'array dei PID
            stato.tempo_rimanente = TEMPO_MANCHE;

            // Pulisce le finestre per la nuova manche
            werase(campo); 
            werase(info); box(campo, 0, 0); 
            box(info, 0, 0); wrefresh(campo); wrefresh(info);
            
            // Creazione delle pipe
            int main_pipe[2], rana_update_pipe[2], child_idx = 0;
            if (pipe(main_pipe) == -1) { 
            	perror("main_pipe"); 
            	exit(1); 
            	}
            if (pipe(rana_update_pipe) == -1) { 
            	perror("rana_update_pipe"); 
            	exit(1); 
            	}

            // --- Creazione dei Processi Figli ---

            // Crea il processo per l'input
            if ((child_pids[child_idx] = fork()) == 0) {
                close(main_pipe[0]); 
                close(rana_update_pipe[0]); 
                close(rana_update_pipe[1]);
                input_process(main_pipe[1]); exit(0);
            } child_idx++;
            
            // Crea il processo per la rana
            if ((child_pids[child_idx] = fork()) == 0) {
                close(main_pipe[0]); 
                close(rana_update_pipe[1]);
                rana_process(main_pipe[1], rana_update_pipe[0]); 
                exit(0);
            } child_idx++;
            
            // Crea i processi per i coccodrilli
            srand(time(NULL));
            int corsia_altezza = (max_y - 2 * ALTEZZA_SPONDA) / NUM_CORSIE;
            
            // Logica di calcolo per la spaziatura dei coccodrilli
            const int MIN_CROC_SPACING = LARGHEZZA_MAX_COC + (LARGHEZZA_RANA * 2); 
            const int MAX_CROC_SPACING = LARGHEZZA_MAX_COC + (LARGHEZZA_RANA * 8);
            for (int i = 0; i < NUM_CORSIE; i++) {
                int corsia_y = ALTEZZA_SPONDA + (i * corsia_altezza);
                int direzione = (i % 2 == 0) ? 1 : -1;
                int total_corsia_distance = 0, croc_positions[NUM_COC];
                for (int j = 0; j < NUM_COC; j++) {
                    croc_positions[j] = total_corsia_distance;
                    int random_spacing = MIN_CROC_SPACING + (rand() % (MAX_CROC_SPACING - MIN_CROC_SPACING + 1));
                    total_corsia_distance += random_spacing;
                }
                int base_distance = (total_corsia_distance > max_x) ? total_corsia_distance : max_x;
                int loop_distance = base_distance + LARGHEZZA_MAX_COC;
                int speed = (rand() % (200000 - 60000 + 1) + 60000);
                for (int j = 0; j < NUM_COC; j++) {
                    int random_croc_larghezza = LARGHEZZA_MIN_COC + (rand() % (LARGHEZZA_MAX_COC - LARGHEZZA_MIN_COC + 1));
                    int start_x = (direzione > 0) ? -random_croc_larghezza - croc_positions[j] : max_x + croc_positions[j];
                    int croc_id = (i * NUM_COC) + j + 1;
                    if ((child_pids[child_idx] = fork()) == 0) {
                        close(main_pipe[0]); close(rana_update_pipe[0]); close(rana_update_pipe[1]);
                        coc_process(main_pipe[1], croc_id, start_x, corsia_y, random_croc_larghezza, direzione, i, loop_distance, speed);
                        exit(0);
                    } child_idx++;
                }
            }

            // Crea il processo per il timer
            if ((child_pids[child_idx] = fork()) == 0) {
                close(main_pipe[0]); 
                close(rana_update_pipe[0]); 
                close(rana_update_pipe[1]);
                timer_process(main_pipe[1]); 
                exit(0);
            } child_idx++;

            // Il processo padre chiude le estremità delle pipe che non usa
            close(main_pipe[1]);
            close(rana_update_pipe[0]);

            // Array locale per memorizzare lo stato del gioco
            GameObject objects[MAX_OBJECTS];
            for(int i=0; i<MAX_OBJECTS; i++) { 
            	objects[i].active = 0; 
            	objects[i].id = -1; 
            	}
            GameObject *frog = &objects[0];
            
            // Setup per la chiamata non bloccante a select()
            fd_set read_fds;
            struct timeval timeout;

            // Ciclo principale della manche (il "motore" del gioco)
            while (!manche_terminata) {
                FD_ZERO(&read_fds); 
                FD_SET(main_pipe[0], &read_fds);
                timeout.tv_sec = 0; 
                timeout.tv_usec = 16000;

                // Usa select() per attendere dati sulla pipe senza bloccare il gioco
                int activity = select(main_pipe[0] + 1, &read_fds, NULL, NULL, &timeout);
                if (activity < 0 && errno != EINTR) { 
                	perror("select"); 
                	break; 
                	}

                // Se ci sono dati da leggere dalla pipe principale
                if (activity > 0 && FD_ISSET(main_pipe[0], &read_fds)) {
                    GameObject obj;
                    ssize_t bytes_read = read_full(main_pipe[0], &obj, sizeof(GameObject));
                    if (bytes_read == sizeof(GameObject)) {
                        // Switch per gestire i diversi tipi di messaggi ricevuti
                        switch(obj.tipo) {
                            case OBJ_INPUT_KEY: { // Gestione dei comandi di input
                                int key = obj.data.input.key_code;
                                if (key == 'q' || key == 'Q') { stato.vite = 0; manche_terminata = true; } 
                                else if (key == ' ' || key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT) {
                                    CommandiRana cmd = {.key = key, .current_pos = frog->pos};
                                    write(rana_update_pipe[1], &cmd, sizeof(CommandiRana));
                                }
                                break;
                            }
                            case OBJ_FROG: // Aggiorna lo stato della rana
                                if (obj.id >= 0 && obj.id < MAX_OBJECTS) {
                                    bool was_riding = objects[obj.id].data.stato_rana.riding_croc;
                                    objects[obj.id] = obj;
                                    objects[obj.id].data.stato_rana.riding_croc = was_riding;
                                }
                                break;
                            case OBJ_CROC: case OBJ_BULLET: case OBJ_GRENADE: // Aggiorna lo stato dei coccodrilli, proiettili e granate
                                if (obj.id >= 0 && obj.id < MAX_OBJECTS) objects[obj.id] = obj;
                                break;
                            case OBJ_TIMER_TICK: // Gestione del timer
                                if (stato.tempo_rimanente >= 0) {
                                    stato.tempo_rimanente--;
                                    if (stato.tempo_rimanente < 0) { stato.vite--; manche_terminata = true; }
                                }
                                break;
                            default: break;
                        }
                    } else if (bytes_read == 0) { manche_terminata = true; }
                }
                
                if (manche_terminata) continue;

                // --- Logica di Gioco e Collisioni ---
                for (int i = 1; i < MAX_OBJECTS; i++) { 
                    GameObject *bullet = &objects[i];
                    if (bullet->active && bullet->tipo == OBJ_BULLET && frog->active &&
                        bullet->pos.x >= frog->pos.x && bullet->pos.x < frog->pos.x + LARGHEZZA_RANA &&
                        bullet->pos.y >= frog->pos.y && bullet->pos.y < frog->pos.y + ALTEZZA_RANA) {
                        kill(bullet->pid, SIGTERM); bullet->active = false;
                        stato.vite--; 
                        manche_terminata = true; 
                        goto next_frame;
                    }
                }
                for (int i = 1; i < MAX_OBJECTS; i++) { 
                    GameObject *grenade = &objects[i];
                    if (!grenade->active || grenade->tipo != OBJ_GRENADE) continue;
                    for (int j = 1; j < MAX_OBJECTS; j++) {
                        GameObject *bullet = &objects[j];
                        if (!bullet->active || bullet->tipo != OBJ_BULLET) continue;
                        if (bullet->pos.y == grenade->pos.y) {
                            bool same_spot = (bullet->pos.x == grenade->pos.x);
                            bool pass_through = (bullet->pos.x == grenade->prev_pos.x && bullet->prev_pos.x == grenade->pos.x);
                            if (same_spot || pass_through) {
                                kill(grenade->pid, SIGTERM); 
                                kill(bullet->pid, SIGTERM);
                                grenade->active = false; 
                                bullet->active = false;
                                stato.punteggio += 10; 
                                goto next_grenade_loop;
                            }
                        }
                    }
                    next_grenade_loop:;
                }
                if (frog->active) { 
                    if (frog->data.stato_rana.riding_croc && frog->data.stato_rana.croc_id != -1) {
                        GameObject* croc = &objects[frog->data.stato_rana.croc_id];
                        if (croc->active && croc->tipo == OBJ_CROC) frog->pos.x = croc->pos.x + frog->data.stato_rana.riding_offset_x;
                        else frog->data.stato_rana.riding_croc = false;
                    }
                    if (frog->pos.y >= ALTEZZA_SPONDA && frog->pos.y < max_y - ALTEZZA_SPONDA) { 
                        bool is_safe = false;
                        int current_corsia = calc_corsia(frog->pos.y, max_y, ALTEZZA_SPONDA, NUM_CORSIE);
                        for (int i = 1; i < MAX_OBJECTS; i++) {
                            GameObject *croc = &objects[i];
                            if (croc->active && croc->tipo == OBJ_CROC && croc->corsia == current_corsia &&
                                frog->pos.x < croc->pos.x + croc->larghezza && frog->pos.x + LARGHEZZA_RANA > croc->pos.x) {
                                if (!frog->data.stato_rana.riding_croc || frog->data.stato_rana.croc_id != croc->id) {
                                    frog->data.stato_rana.riding_croc = true;
                                    frog->data.stato_rana.croc_id = croc->id;
                                    frog->data.stato_rana.riding_offset_x = frog->pos.x - croc->pos.x;
                                }
                                is_safe = true; 
                                frog->pos.y = croc->pos.y; 
                                break;
                            }
                        }
                        if (!is_safe) { 
                        	stato.vite--; 
                        	manche_terminata = true; 
                        	goto next_frame; 
                        	}
                    } else { 
                        frog->data.stato_rana.riding_croc = false; 
                        frog->data.stato_rana.croc_id = -1;
                        if (frog->pos.y <= posizioni_tane[0].y) { 
                            for (int i = 0; i < NUM_TANE; i++) {
                                if (frog->pos.x < posizioni_tane[i].x + 7 && frog->pos.x + LARGHEZZA_RANA > posizioni_tane[i].x) {
                                    if (stato.tane_occupate[i] == 0) {
                                        stato.tane_occupate[i] = 1; 
                                        stato.punteggio += 100;
                                        bool tutte_occupate = true;
                                        for (int j = 0; j < NUM_TANE; j++) if (stato.tane_occupate[j] == 0) tutte_occupate = false;
                                        if (tutte_occupate) goto gioco_finito;
                                    } else { stato.vite--; }
                                    manche_terminata = true; 
                                    goto next_frame;
                                }
                            }
                        }
                    }
                }
                if (frog->active && (frog->pos.x < 0 || frog->pos.x + LARGHEZZA_RANA > max_x)) {
                    stato.vite--; 
                    manche_terminata = true;
                }

                next_frame:;
                werase(campo); box(campo, 0, 0); draw_bordi(); draw_tane();
                for (int i = 1; i < MAX_OBJECTS; i++) {
                    if(objects[i].active) {
                        switch(objects[i].tipo) {
                            case OBJ_CROC: draw_coc(&objects[i]); 
                            	break;
                            case OBJ_BULLET: draw_proiettile(&objects[i]); 
                            	break;
                            case OBJ_GRENADE: draw_granata(&objects[i]); 
                            	break;
                            default: 
                            	break;
                        }
                    }
                }
                if (frog->active) draw_rana(frog);
                werase(info); 
                mvwprintw(info, 1, 2, "Tempo: %d  Vite: %d  Score: %d", stato.tempo_rimanente > 0 ? stato.tempo_rimanente : 0, stato.vite, stato.punteggio);
                wrefresh(campo); wrefresh(info);
            }
            cleanup_processes();
            close(main_pipe[0]);
            close(rana_update_pipe[1]);
        }

    gioco_finito:;
        cleanup_processes();
        bool vittoria = true;
        for(int i = 0; i < NUM_TANE; i++) if (stato.tane_occupate[i] == 0) vittoria = false;
        if(stato.vite <= 0 && !vittoria) vittoria = false;
        play_again = draw_fine(vittoria, false);

    } while (play_again);

    // Cleanup finale di ncurses
    delwin(score); 
    delwin(campo); 
    delwin(info);
    endwin();
    return 0;
}
