#include "graphics.h"

// Disegna un proiettile sullo schermo
void draw_proiettile(GameObject *projectile) {
    // Non disegna nulla se l'oggetto è inattivo
    if (!projectile->active) {
        return;
    }
    // Imposta il colore e disegna il carattere del proiettile
    wattron(campo, COLOR_PAIR(CP_PROJECTILE) | A_BOLD);
    mvwaddch(campo, projectile->pos.y, projectile->pos.x, 'o');
    wattroff(campo, COLOR_PAIR(CP_PROJECTILE) | A_BOLD);
}

// Disegna una granata sparata dalla rana
void draw_granata(GameObject *grenade) {
    if (!grenade->active) {
        return;
    }
    // Imposta il colore e usa un carattere a diamante per la granata
    wattron(campo, COLOR_PAIR(CP_GRENADE) | A_BOLD);
    mvwaddch(campo, grenade->pos.y, grenade->pos.x, ACS_DIAMOND);
    wattroff(campo, COLOR_PAIR(CP_GRENADE) | A_BOLD);
}

// Disegna la rana del giocatore
void draw_rana(GameObject *frog) {
    if (!frog->active) {
        return;
    }
    int x = frog->pos.x;
    int y = frog->pos.y;

    // Disegna il corpo superiore della rana
    wattron(campo, A_BOLD | COLOR_PAIR(CP_RANA));
    mvwaddch(campo, y, x, ACS_CKBOARD);
    mvwaddch(campo, y, x + 1, ACS_CKBOARD);
    mvwaddch(campo, y, x + 2, ACS_CKBOARD);
    wattroff(campo, A_BOLD | COLOR_PAIR(CP_RANA));

    // Disegna gli occhi con un colore diverso per risaltare
    wattron(campo, A_BOLD | COLOR_PAIR(CP_EYES));
    mvwaddch(campo, y + 1, x, ACS_BULLET);
    mvwaddch(campo, y + 1, x + 2, ACS_BULLET);
    wattroff(campo, A_BOLD | COLOR_PAIR(CP_EYES));

    // Completa il corpo inferiore
    wattron(campo, A_BOLD | COLOR_PAIR(CP_RANA));
    mvwaddch(campo, y + 1, x + 1, ACS_CKBOARD);
    wattroff(campo, A_BOLD | COLOR_PAIR(CP_RANA));
}

// Disegna un singolo coccodrillo
void draw_coc(GameObject *croc) {
    if (!croc->active) {
        return;
    }
    int x = croc->pos.x;
    int y = croc->pos.y;

    // Disegna il corpo principale del coccodrillo
    wattron(campo, COLOR_PAIR(CP_COC) | A_BOLD);
    for (int i = 0; i < croc->larghezza; i++) {
        mvwaddch(campo, y, x + i, ACS_CKBOARD);
    }
    // Disegna gli occhi stilizzati sulla fila inferiore
    mvwaddch(campo, y + 1, x, ACS_BULLET);
    for (int i = 1; i < croc->larghezza - 1; i++) {
        mvwaddch(campo, y + 1, x + i, ACS_CKBOARD);
    }
    mvwaddch(campo, y + 1, x + croc->larghezza - 1, ACS_BULLET);
    wattroff(campo, COLOR_PAIR(CP_COC) | A_BOLD);
}

// Disegna le sponde superiore e inferiore
void draw_sponde(void) {
    // Sponda superiore
    wattron(campo, COLOR_PAIR(CP_ERBA));
    for (int y = 0; y < ALTEZZA_SPONDA; y++) {
        for (int x = 0; x < max_x; x++) {
            mvwaddch(campo, y, x, ACS_CKBOARD);
        }
    }
    wattroff(campo, COLOR_PAIR(CP_ERBA));

    // Sponda inferiore
    wattron(campo, COLOR_PAIR(CP_PAVIMENTO));
    for (int y = max_y - ALTEZZA_SPONDA; y < max_y; y++) {
        for (int x = 0; x < max_x; x++) {
            mvwaddch(campo, y, x, ACS_CKBOARD);
        }
    }
    wattroff(campo, COLOR_PAIR(CP_PAVIMENTO));
}

// Disegna le tane di arrivo della rana
void draw_tane(void) {
    int start_x_tane = 6;
    int y_tana = 1;
    for (int i = 0; i < NUM_TANE; i++) {
        int x_pos_tana = start_x_tane + i * (LARGHEZZA_RANA + LARGHEZZA_RANA + 10);
        // Se la tana è occupata, disegna una rana stilizzata
        if (stato.tane_occupate[i] == 1) {
            wattron(campo, COLOR_PAIR(CP_TANA_CHIUSA) | A_BOLD);
            for (int j = 0; j < ALTEZZA_RANA; j++) {
                mvwprintw(campo, y_tana - j, x_pos_tana, "|%c%c%c%c|", ACS_DIAMOND, ACS_DIAMOND, ACS_DIAMOND, ACS_DIAMOND);
            }
            wattroff(campo, COLOR_PAIR(CP_TANA_CHIUSA) | A_BOLD);
        } else { // Altrimenti disegna una tana vuota
            wattron(campo, COLOR_PAIR(CP_TANA));
            for (int j = 0; j < ALTEZZA_RANA; j++) {
                mvwprintw(campo, y_tana - j, x_pos_tana, "|     |");
            }
            wattroff(campo, COLOR_PAIR(CP_TANA));
        }
    }
}

// Mostra la schermata di fine gioco e gestisce la scelta di rigiocare
bool draw_fine(bool vittoria, bool quit) {
    werase(campo);
    box(campo, 0, 0);

    // Seleziona il messaggio appropriato
    const char *title;
    if (quit) {
        title = "HAI ABBANDONATO";
    } else if (vittoria) {
        title = "HAI VINTO!";
    } else {
        title = "GAME OVER";
    }

    // Prepara i testi da visualizzare
    char score_text[50];
    sprintf(score_text, "Punteggio finale: %d", stato.punteggio);
    const char *prompt_text = "Giocare di nuovo? [S]i / [N]o";

    // Centra e stampa i testi
    int mid_y = max_y / 2;
    int mid_x = max_x / 2;

    wattron(campo, A_BOLD);
    mvwprintw(campo, mid_y - 2, mid_x - strlen(title) / 2, "%s", title);
    wattroff(campo, A_BOLD);

    mvwprintw(campo, mid_y, mid_x - strlen(score_text) / 2, "%s", score_text);
    mvwprintw(campo, mid_y + 2, mid_x - strlen(prompt_text) / 2, "%s", prompt_text);
    wrefresh(campo);

    // Attende l'input dell'utente in modo bloccante
    nodelay(stdscr, FALSE);
    int choice;
    while (true) {
        choice = getch();
        if (choice == 's' || choice == 'S') {
            nodelay(stdscr, TRUE); // Riabilita la modalità non bloccante per il gioco
            return true;
        }
        if (choice == 'n' || choice == 'N' || choice == 'q' || choice == 'Q') {
            nodelay(stdscr, TRUE);
            return false;
        }
    }
}