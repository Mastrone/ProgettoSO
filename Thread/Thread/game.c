#include "common.h"
#include "utils.h"
#include "graphics.h"
#include "game.h"

// Inizializza le variabili di stato del gioco
void init_game(void) {
    stato.punteggio = 0;
    stato.vite = VITE_INIZIALI;
    stato.tempo_rimanente = TEMPO_MANCHE;
    for (int i = 0; i < NUM_TANE; i++) {
        stato.tane_occupate[i] = 0;
    }

    // Calcola la posizione delle tane
    int start_x_tane = 6;
    for (int i = 0; i < NUM_TANE; i++) {
        posizioni_tane[i].x = start_x_tane + i * (2 * LARGHEZZA_RANA + 10);
        posizioni_tane[i].y = 1;
    }
}


// Thread per la gestione dell'input dell'utente
void *input_thread(void *arg) {
    while (!game_over) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q') { // Se l'utente preme 'q' termina il gioco
                user_quit = true;
                game_over = true;
            } else if (ch == ' ') { // Se l'utente preme spazio lancia una granata
                pthread_mutex_lock(&input_mutex);
                granata_sparata = true;
                pthread_mutex_unlock(&input_mutex);
            } else if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                pthread_mutex_lock(&input_mutex);
                input_key = ch;
                pthread_mutex_unlock(&input_mutex);
            }
        }
        usleep(500);
    }
    return NULL;
}

// Thread produttore che gestisce la rana
void *rana_thread(void *arg) {
    GameObject frog = {
        // Stato iniziale della rana
        .tipo = OBJ_RANA, .id = 0, .pos = {max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2},
        .prev_pos = {max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2}, .larghezza = LARGHEZZA_RANA,
        .altezza = ALTEZZA_RANA, .direzione = 0, .corsia = -1, .active = 1,
        .riding_croc = false, .croc_id = -1
    };
    produce(&buffer, frog); // Invia dati rana iniziali al consumatore

    while (!game_over) {
        pthread_mutex_lock(&mutex_pos_rana);
        if (pos_r_update) {
            frog.pos = pos_r; // Aggiorna posizione interna della rana
            frog.prev_pos = pos_r;
            pos_r_update = false;
        }
        pthread_mutex_unlock(&mutex_pos_rana);

        // Legge input utente in modo sicuro con mutex
        pthread_mutex_lock(&input_mutex);
        int key = input_key;
        bool sparare = granata_sparata;
        input_key = 0;
        granata_sparata = false;
        pthread_mutex_unlock(&input_mutex);

        // Se l'utente ha sparato, crea due thread per le granate (sx e dx)
        if (sparare) {
            ProiettileArgs *param_sx = malloc(sizeof(ProiettileArgs));
            if (param_sx) {
                param_sx->start_pos.x = frog.pos.x - 1;
                param_sx->start_pos.y = frog.pos.y + 1;
                param_sx->direzione = -1;
                pthread_t t_grenade_left;
                pthread_create(&t_grenade_left, NULL, grenade_thread, param_sx);
            }
            ProiettileArgs *param_dx = malloc(sizeof(ProiettileArgs));
            if (param_dx) {
                param_dx->start_pos.x = frog.pos.x + frog.larghezza;
                param_dx->start_pos.y = frog.pos.y + 1;
                param_dx->direzione = 1;
                pthread_t t_grenade_right;
                pthread_create(&t_grenade_right, NULL, grenade_thread, param_dx);
            }
        }

        // Se l'utente si è mosso
        if (key) {
            frog.prev_pos = frog.pos; // Aggiorna la posizione precedente della rana
            switch (key) {
                case KEY_UP: // Salto in alto
                    frog.pos.y -= ALTEZZA_RANA;
                    frog.riding_croc = false; // Il salto verticale interrompre il trasporto sul coccodrillo
                    frog.croc_id = -1;
                    break;
                case KEY_DOWN: // Salto in basso
                    frog.pos.y += ALTEZZA_RANA;
                    frog.riding_croc = false;
                    frog.croc_id = -1;
                    break;
                case KEY_LEFT: // Movimento a sinistra
                    frog.pos.x -= LARGHEZZA_RANA;
                    break;
                case KEY_RIGHT: // Movimento a destra
                    frog.pos.x += LARGHEZZA_RANA;
                    break;
            }

            // Controllo che la rana non esca dai bordi del campo
            if (frog.pos.x < 0) {
                frog.pos.x = 0;
            } else if (frog.pos.x > max_x - LARGHEZZA_RANA) {
                frog.pos.x = max_x - LARGHEZZA_RANA;
            }
            if (frog.pos.y < 0) {
                frog.pos.y = 0;
            } else if (frog.pos.y > max_y - ALTEZZA_RANA) {
                frog.pos.y = max_y - ALTEZZA_RANA;
            }
            produce(&buffer, frog); // Invia la nuova rana al consumatore
        }
        usleep(500);
    }
    return NULL;
}

// Thread produttore per un singolo coccodrillo.
void *croc_thread(void *arg) {
    // Recupera i parametri passati al thread
    CocArgs *args = (CocArgs *)arg;
    int start_x = args->start_x;
    int direzione = args->direzione;
    int velocita = args->velocita;
    int larghezza = args->larghezza;
    int distanza_loop = args->distanza_loop;
    int croc_id = args->id;
    int corsia = args->corsia;

    // Calcola la posizione Y della corsia
    int lane_altezza = (max_y - 2 * ALTEZZA_SPONDA) / NUM_CORSIE;
    int lane_y = ALTEZZA_SPONDA + (corsia * lane_altezza);

    free(arg); 

    // Stato iniziale del coccodrillo
    GameObject coc = {
        .tipo = OBJ_COC,
        .id = croc_id,
        .pos = {.x = start_x, .y = lane_y},
        .prev_pos = {.x = start_x, .y = lane_y},
        .larghezza = larghezza, .altezza = 2, .direzione = direzione,
        .corsia = corsia, .active = 1
    };

    while (!game_over) {
        coc.prev_pos = coc.pos;
        coc.pos.x += coc.direzione;

        // Controlla se il coccodrillo ha raggiunto i bordi del campo
        if (direzione > 0 && coc.pos.x >= max_x) {
            coc.pos.x -= distanza_loop;
        } else if (direzione < 0 && (coc.pos.x + coc.larghezza) <= 0) {
            coc.pos.x += distanza_loop;
        }

        produce(&buffer, coc);

        // Spara un proiettile con una certa probabilità
        if (rand() % PROB_PROIETTILE == 0) {
            ProiettileArgs *p_args = malloc(sizeof(ProiettileArgs));
            if (p_args) {
                p_args->start_pos.x = (direzione > 0) ? coc.pos.x + coc.larghezza : coc.pos.x - 1;
                p_args->start_pos.y = coc.pos.y + 1;
                p_args->direzione = coc.direzione;

                pthread_t t_projectile;
                if (pthread_create(&t_projectile, NULL, projectile_thread, p_args) == 0) {
                    pthread_detach(t_projectile);
                } else {
                    free(p_args);
                }
            }
        }

        usleep(velocita);
    }
    return NULL;
}

// Thread produttore per una singola granata
void *grenade_thread(void *arg) {
    pthread_detach(pthread_self()); // Il thread pulirà le sue risorse alla fine
    ProiettileArgs *args = (ProiettileArgs *)arg;
    Posizione start_pos = args->start_pos;
    int direzione = args->direzione;
    free(arg);

    // Assegna un ID univoco per la granata
    pthread_mutex_lock(&dynamic_object_id_mutex);
    int grenade_id = next_dynamic_object_id++;
    pthread_mutex_unlock(&dynamic_object_id_mutex);

    GameObject grenade = {
        .tipo = OBJ_GRANATA, .id = grenade_id, .pos = start_pos, .prev_pos = start_pos,
        .larghezza = 1, .altezza = 1, .direzione = direzione, .corsia = -1, .active = 1
    };

    while (!game_over  && !manche_terminata) {
        grenade.prev_pos = grenade.pos;
        grenade.pos.x += grenade.direzione;
        // Se esce dallo schermo il ciclo termina
        if (grenade.pos.x < 0 || grenade.pos.x >= max_x) {
            break;
        }
        produce(&buffer, grenade);
        usleep(RITARDO_GRANATA);
    }
    // Invia un ultimo messaggio per notificare la sua disattivazione
    grenade.active = 0;
    produce(&buffer, grenade);
    return NULL;
}


// Thread produttore per un singolo proiettile
void *projectile_thread(void *arg) {
    pthread_detach(pthread_self());
    ProiettileArgs *args = (ProiettileArgs *)arg;
    Posizione start_pos = args->start_pos;
    int direzione = args->direzione;
    free(arg);

    pthread_mutex_lock(&dynamic_object_id_mutex);
    int projectile_id = next_dynamic_object_id++;
    pthread_mutex_unlock(&dynamic_object_id_mutex);

    GameObject projectile = {
        .tipo = OBJ_PROIETTILE, .id = projectile_id, .pos = start_pos, .prev_pos = start_pos,
        .larghezza = 1, .altezza = 1, .direzione = direzione, .corsia = -1, .active = 1
    };

    while (!game_over  && !manche_terminata ) {
        projectile.prev_pos = projectile.pos;
        projectile.pos.x += projectile.direzione;
        if (projectile.pos.x < 0 || projectile.pos.x >= max_x) {
            break;
        }
        produce(&buffer, projectile);
        usleep(RITARDO_PROIETTILE);
    }

    projectile.active = 0;
    produce(&buffer, projectile);
    return NULL;
}

// Thread per il timer di gioco
void *game_timer(void *arg) {
    while (!game_over && !manche_terminata) {
        sleep(1);
        
        if (stato.tempo_rimanente > 0) {
           stato.tempo_rimanente--;
        }

        // Se il tempo scade, termina la manche.
        if (stato.tempo_rimanente <= 0 && !manche_terminata) {
            manche_terminata = true; 
        }
    }
    return NULL;
}

// Thread consumatore
void *consumer_thread(void *arg) {
    // Array locale che rappresenta lo stato di tutti gli oggetti nel mondo di gioco
    GameObject objects[NUM_MAX_OGGETTI];
    for (int i = 0; i < NUM_MAX_OGGETTI; i++) {
        objects[i].active = 0;
        objects[i].id = -1; // ID non valido per marcare slot vuoti.
    }

    GameObject *frog = &objects[0]; // Alias per la rana per una maggiore leggibilità

    while (!game_over && !manche_terminata && !livello_vinto) {
        GameObject obj = consume(&buffer); // Estrae un aggiornamento dal buffer

        if (obj.id < 0 || obj.id >= NUM_MAX_OGGETTI) {
            continue; // Ignora oggetti con ID non validi
        }
        // Filtro ignora aggiornamenti di proiettili/granate già distrutti
        if (objects[obj.id].id == obj.id && objects[obj.id].active == 0 &&
            (objects[obj.id].tipo == OBJ_PROIETTILE || objects[obj.id].tipo == OBJ_GRANATA))
        {
            continue;
        }

        // Aggiorna lo stato dell'oggetto
        objects[obj.id] = obj;

        // --- Logica di collisione e stato (eseguita a ogni ciclo) ---
        
        // Controlla collisione granata-proiettile.
        if (obj.tipo == OBJ_GRANATA && obj.active) {
            for (int i = 1; i < NUM_MAX_OGGETTI; i++) {
                GameObject *projectile = &objects[i];
                if (projectile->active && projectile->tipo == OBJ_PROIETTILE) {
                    // La collisione avviene se sono sulla stessa riga e si "toccano"
                    if (obj.pos.y == projectile->pos.y && abs(obj.pos.x - projectile->pos.x) <= 1)
                    {
                        objects[obj.id].active = 0; // Disattiva entrambi
                        projectile->active = 0;
                        break;
                    }
                }
            }
        }
        // Controllo duplicato per coprire il caso in cui il proiettile si muove contro una granata ferma
        else if (obj.tipo == OBJ_PROIETTILE && obj.active) {
            for (int i = 1; i < NUM_MAX_OGGETTI; i++) {
                GameObject *grenade = &objects[i];
                if (grenade->active && grenade->tipo == OBJ_GRANATA) {
                    if (obj.pos.y == grenade->pos.y && abs(obj.pos.x - grenade->pos.x) <= 1)
                    {
                        objects[obj.id].active = 0;
                        grenade->active = 0;
                        break;
                    }
                }
            }
        }

        // Logica specifica per il trasporto della rana da parte di un coccodrillo
        if (obj.tipo == OBJ_COC) {
            if (frog->active && frog->riding_croc && frog->croc_id == obj.id) {
                int movement_x = obj.pos.x - obj.prev_pos.x;
                frog->prev_pos = frog->pos;
                frog->pos.x += movement_x; // La rana si muove con il coccodrillo.
                // Se viene trasportata fuori schermo, perde una vita.
                if (frog->pos.x < 0 || frog->pos.x > max_x - frog->larghezza) {
                    manche_terminata = true;
                    continue;
                }
                
                pthread_mutex_lock(&mutex_pos_rana);
                pos_r = frog->pos;
                pos_r_update = true;
                pthread_mutex_unlock(&mutex_pos_rana);
            }
        } 
        // Logica specifica per lo stato della rana
        else if (obj.tipo == OBJ_RANA) {
            // Se la rana è nel fiume...
            if (frog->pos.y >= ALTEZZA_SPONDA && frog->pos.y < max_y - ALTEZZA_SPONDA) {
                bool on_croc = false;
                int current_lane = calc_corsia(frog->pos.y, max_y, ALTEZZA_SPONDA, NUM_CORSIE);
                // controlla se è su un coccodrillo.
                for (int i = 1; i < NUM_MAX_OGGETTI; i++) {
                    GameObject *croc = &objects[i];
                    if (croc->active && croc->tipo == OBJ_COC && croc->corsia == current_lane) {
                        if (frog->pos.x + frog->larghezza > croc->pos.x && frog->pos.x < croc->pos.x + croc->larghezza) {
                            on_croc = true;
                            frog->riding_croc = true;
                            frog->croc_id = croc->id;
                            frog->pos.y = croc->pos.y; // Allineamento verticale
                            break;
                        }
                    }
                }
                // Se non è su un coccodrillo annega
                if (!on_croc) {
                    manche_terminata = true;
                    continue;
                }
            } 
            // Se la rana è sulle sponde
            else {
                if (frog->riding_croc) {
                    frog->riding_croc = false;
                    frog->croc_id = -1;
                }
                // controlla se è in una tana
                if (frog->pos.y < ALTEZZA_SPONDA) {
                    for (int i = 0; i < NUM_TANE; i++) {
                        if (stato.tane_occupate[i] == 0 && frog->pos.x >= posizioni_tane[i].x && frog->pos.x < posizioni_tane[i].x + LARGHEZZA_RANA && frog->pos.y <= posizioni_tane[i].y) {
                            stato.tane_occupate[i] = 1;
                            stato.punteggio += 100;
                            // Resetta la posizione della rana per la manche successiva
                            frog->pos = (Posizione){max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2};
                            break;
                        }
                    }
                    // Controlla se tutte le tane sono piene (vittoria)
                    bool tutte_occupate = true;
                    for (int i = 0; i < NUM_TANE; i++) {
                        if (stato.tane_occupate[i] == 0) tutte_occupate = false;
                    }
                    if (tutte_occupate) {
                        livello_vinto = 1;
                        stato.punteggio += 1000;
                        continue;
                    }
                }
            }
            pthread_mutex_lock(&mutex_pos_rana);
            pos_r = frog->pos;
            pos_r_update = true;
            pthread_mutex_unlock(&mutex_pos_rana);
        }

        // Controlla la collisione tra la rana e i proiettili
        if (frog->active) {
            for (int i = 1; i < NUM_MAX_OGGETTI; i++) {
                GameObject *proj = &objects[i];
                if (proj->active && proj->tipo == OBJ_PROIETTILE) {
                    if (proj->pos.x >= frog->pos.x && proj->pos.x < frog->pos.x + frog->larghezza &&
                        proj->pos.y >= frog->pos.y && proj->pos.y < frog->pos.y + frog->altezza) {

                        proj->active = 0;
                        manche_terminata = true;
                        break;
                    }
                }
            }
        }

        if (manche_terminata || livello_vinto) continue;

        // --- Sezione di Disegno ---
        werase(campo);
        box(campo, 0, 0);
        draw_sponde();
        draw_tane();

        // Disegna tutti gli oggetti attivi
        for (int i = 1; i < NUM_MAX_OGGETTI; i++) {
            if (objects[i].active) {
                if (objects[i].tipo == OBJ_COC) draw_coc(&objects[i]);
                else if (objects[i].tipo == OBJ_GRANATA) draw_granata(&objects[i]);
                else if (objects[i].tipo == OBJ_PROIETTILE) draw_proiettile(&objects[i]);
            }
        }
        if (frog->active) draw_rana(frog);

        // Aggiorna la finestra delle informazioni
        werase(info);
        mvwprintw(info, 1, 2, "Tempo: %d  Vite: %d  Score: %d", stato.tempo_rimanente, stato.vite, stato.punteggio);
        wrefresh(campo);
        wrefresh(info);
        usleep(1000);
    }
    return NULL;
}

