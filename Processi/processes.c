#include "processes.h"
#include "utils.h"

// Processo figlio per la gestione dell'input
void input_process(int pipe_write_fd) {
    signal(SIGTERM, handle_sigterm_child); // Imposta come terminare se riceve SIGTERM
    int ch;
    GameObject key_msg = {.tipo = OBJ_INPUT_KEY};
    while (1) {
        ch = getch(); // Legge un carattere in modo non bloccante
        if (ch != ERR) {
            key_msg.data.input.key_code = ch;
            // Scrive il tasto premuto sulla pipe principale per il processo padre
            if (write(pipe_write_fd, &key_msg, sizeof(GameObject)) == -1) break;
        }
        usleep(10000); // Pausa per ridurre l'uso della CPU
    }
    exit(1);
}

// Processo figlio per la gestione della logica della rana
void rana_process(int pipe_write_fd, int pipe_read_update_fd) {
    signal(SIGTERM, handle_sigterm_child);
    // Rende la pipe di aggiornamento non bloccante
    fcntl(pipe_read_update_fd, F_SETFL, O_NONBLOCK);

    // Stato iniziale della rana
    GameObject frog = {
        .tipo = OBJ_FROG, .id = 0, .pid = getpid(), 
        .pos = {max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2},
        .prev_pos = {max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2}, .larghezza = LARGHEZZA_RANA,
        .altezza = ALTEZZA_RANA, .direzione = 0, .corsia = -1, .active = 1,
        .data.stato_rana = {.riding_croc = false, .croc_id = -1, .riding_offset_x = 0}
    };
    
    // Contatore per gli ID delle granate
    static int grenade_id_counter = MAX_OBJECTS - 100;
    
    // Invia lo stato iniziale al processo padre
    if (write(pipe_write_fd, &frog, sizeof(GameObject)) == -1) exit(1);

    while (1) {
        CommandiRana command;
        // Controlla se ci sono comandi dal processo padre (es. movimento o reset)
        if (read(pipe_read_update_fd, &command, sizeof(CommandiRana)) > 0) {
            frog.pos = command.current_pos; // Sincronizza la posizione con il padre

            if (command.key == ' ') { // Se il comando è 'spara'
                // Crea un nuovo processo per ogni granata
                if (fork() == 0) {
                    Posizione start_pos = {frog.pos.x + LARGHEZZA_RANA, frog.pos.y};
                    granata_process(pipe_write_fd, grenade_id_counter, frog.id, start_pos, 1);
                    exit(0);
                }
                if (fork() == 0) {
                    Posizione start_pos = {frog.pos.x - 1, frog.pos.y};
                    granata_process(pipe_write_fd, grenade_id_counter + 1, frog.id, start_pos, -1);
                    exit(0);
                }
                grenade_id_counter += 2; // Avanza il contatore degli ID
                if(grenade_id_counter >= MAX_OBJECTS - 2) grenade_id_counter = MAX_OBJECTS - 100;
            } else if (command.key == -1) { // Comando speciale per il reset
                frog.pos = (Posizione){max_x / 2 - LARGHEZZA_RANA / 2, max_y - ALTEZZA_SPONDA + 2};
                frog.data.stato_rana.riding_croc = false;
                frog.data.stato_rana.croc_id = -1;
            } else { // Comando di movimento
                frog.prev_pos = frog.pos; 
                switch (command.key) {
                    case KEY_UP: frog.pos.y -= ALTEZZA_RANA; frog.data.stato_rana.riding_croc = false; break;
                    case KEY_DOWN: frog.pos.y += ALTEZZA_RANA; frog.data.stato_rana.riding_croc = false; break;
                    case KEY_LEFT: frog.pos.x -= LARGHEZZA_RANA; break;
                    case KEY_RIGHT: frog.pos.x += LARGHEZZA_RANA; break;
                }
                // Controlla i bordi dello schermo
                if (frog.pos.x < 0) frog.pos.x = 0; else if (frog.pos.x > max_x - LARGHEZZA_RANA) frog.pos.x = max_x - LARGHEZZA_RANA;
                if (frog.pos.y < 0) frog.pos.y = 0; else if (frog.pos.y > max_y - ALTEZZA_RANA) frog.pos.y = max_y - ALTEZZA_RANA;
            }
            // Invia lo stato aggiornato della rana al processo padre
            if (write(pipe_write_fd, &frog, sizeof(GameObject)) == -1) break;
        }
        usleep(16000);
    }
    exit(1);
}

// Processo figlio che gestisce un singolo coccodrillo
void coc_process(int pipe_write_fd, int id, int start_x, int y, int larghezza, int direzione, int corsia, int loop_distance, int speed) {
    signal(SIGTERM, handle_sigterm_child);
    srand(time(NULL) ^ getpid()); // Inizializza il seme casuale per questo processo

    GameObject croc = {
        .tipo = OBJ_CROC, .id = id, .pid = getpid(),
        .pos = {start_x, y}, .prev_pos = {start_x, y},
        .larghezza = larghezza, .altezza = 2, .direzione = direzione, .corsia = corsia, .active = 1
    };
    bool bullet_active = false;
    pid_t bullet_pid = 0;

    if (write(pipe_write_fd, &croc, sizeof(GameObject)) == -1) exit(1);

    while (1) {
        croc.prev_pos = croc.pos;
        croc.pos.x += croc.direzione;

        // Gestione del "wrapping" orizzontale
        if (direzione > 0 && croc.pos.x > max_x) {
            croc.pos.x -= loop_distance;
        } else if (direzione < 0 && (croc.pos.x + croc.larghezza) < 0) {
            croc.pos.x += loop_distance;
        }

        if (write(pipe_write_fd, &croc, sizeof(GameObject)) == -1) break;

        // Controlla se un processo proiettile figlio è terminato
        if (bullet_active) {
            int status;
            pid_t result = waitpid(bullet_pid, &status, WNOHANG);
            if (result == bullet_pid) {
                bullet_active = false; 
                bullet_pid = 0;
            }
        }

        // Spara un proiettile con una certa probabilità se non ne ha già uno attivo
        if (!bullet_active && (rand() % PROB_PROIETTILE == 0)) {
            bullet_active = true;
            if ((bullet_pid = fork()) == 0) {
                // Calcola la posizione di partenza del proiettile
                int bullet_start_x;
                if (direzione > 0) bullet_start_x = croc.pos.x + croc.larghezza;
                else bullet_start_x = croc.pos.x - 1;
                Posizione bullet_start = {bullet_start_x, croc.pos.y};
                int bullet_id = id + (MAX_OBJECTS / 2); 
                proettile_process(pipe_write_fd, bullet_id, id, bullet_start, direzione);
                exit(0); // Il processo figlio proiettile termina qui
            }
        }
        usleep(speed);
    }
    exit(1);
}

// Processo figlio per il timer di gioco
void timer_process(int pipe_write_fd) {
    signal(SIGTERM, handle_sigterm_child);
    GameObject timer_tick = {.tipo = OBJ_TIMER_TICK};
    while (1) {
        sleep(1); // Invia un "tick" ogni secondo
        if (write(pipe_write_fd, &timer_tick, sizeof(GameObject)) == -1) break;
    }
    exit(1);
}

// Processo figlio che gestisce un singolo proiettile
void proettile_process(int pipe_write_fd, int id, int owner_id, Posizione start_pos, int direzione) {
    signal(SIGTERM, handle_sigterm_child);
    GameObject bullet = {
        .tipo = OBJ_BULLET, .id = id, .pid = getpid(), 
        .pos = start_pos, .prev_pos = start_pos,
        .larghezza = 1, .altezza = 1, .direzione = direzione, .active = 1, .owner_id = owner_id
    };
    
    while(bullet.active) {
        bullet.prev_pos = bullet.pos;
        bullet.pos.x += bullet.direzione;

        // Se esce dallo schermo, si disattiva
        if (bullet.pos.x < 0 || bullet.pos.x >= max_x) bullet.active = 0;
        if (write(pipe_write_fd, &bullet, sizeof(GameObject)) == -1) break;
        if (!bullet.active) break;
        
        usleep(RITARDO_PROIETTILE);
    }
    exit(0); // Il processo termina dopo aver inviato il suo stato finale "inattivo"
}

// Processo figlio che gestisce una singola granata
void granata_process(int pipe_write_fd, int id, int owner_id, Posizione start_pos, int direzione) {
    signal(SIGTERM, handle_sigterm_child);
    GameObject grenade = {
        .tipo = OBJ_GRENADE, .id = id, .pid = getpid(),
        .pos = start_pos, .prev_pos = start_pos,
        .larghezza = 1, .altezza = 1, .direzione = direzione, .active = 1, .owner_id = owner_id
    };

    while(grenade.active) {
        grenade.prev_pos = grenade.pos;
        grenade.pos.x += grenade.direzione;

        if (grenade.pos.x < 0 || grenade.pos.x >= max_x) grenade.active = 0;
        if (write(pipe_write_fd, &grenade, sizeof(GameObject)) == -1) break;
        if (!grenade.active) break;
        
        usleep(RITARDO_GRANATA);
    }
    exit(0); // Il processo termina dopo l'ultimo aggiornamento
}