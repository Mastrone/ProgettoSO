#include "common.h"
#include "utils.h"
#include "graphics.h"
#include "game.h"

// Definizione delle variabili globali
GameObjectBuffer buffer;
pthread_t coc_threads[NUM_CORSIE];
StatoGioco stato;
Posizione posizioni_tane[NUM_TANE];
int max_x, max_y;
volatile bool game_over = false;
volatile int input_key = 0;
volatile bool user_quit = false;
volatile bool granata_sparata = false;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool manche_terminata = false;
volatile bool livello_vinto = false;
pthread_mutex_t mutex_pos_rana = PTHREAD_MUTEX_INITIALIZER;
Posizione pos_r;
bool pos_r_update = false;

// Gestione ID per oggetti dinamici (granate, proiettili)
pthread_mutex_t dynamic_object_id_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_dynamic_object_id = 100;

// Puntatori alle finestre di ncurses
WINDOW *score, *campo, *info;

// Signal handler per la chiusura pulita con Ctrl+C
void handle_sigint(int sig) {
    game_over = true;
    endwin();
    exit(0);
}

int main() {
    // Impostazioni iniziali del programma
    system("echo -ne '\e[8;33;80t'"); // Ridimensiona il terminale a 40 righe e 80 colonne
    usleep(100000);
    signal(SIGINT, handle_sigint);
    

    // Inizializzazione di ncurses
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); //getch() non bloccante
    start_color();

    // Creazione delle finestre di gioco
    score = newwin(2, COLS, 0, 0);
    campo = newwin(28, COLS, 2, 0);
    info = newwin(2, COLS, 30, 0);

    init_pair(CP_RANA, COLOR_WHITE, COLOR_GREEN);
    init_pair(CP_TANA_CHIUSA, COLOR_BLACK, COLOR_WHITE);
    init_pair(CP_ERBA, COLOR_YELLOW, COLOR_GREEN);
    init_pair(CP_PAVIMENTO, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CP_EYES, COLOR_YELLOW, COLOR_GREEN);
    init_pair(CP_SCORE_BG, COLOR_WHITE, COLOR_BLACK);
    init_pair(CP_INFO, COLOR_WHITE, COLOR_BLACK);
    init_pair(CP_FIELD_BG, COLOR_WHITE, COLOR_BLUE);
    init_pair(CP_TANA, COLOR_WHITE, COLOR_GREEN);
    init_pair(CP_COC, COLOR_BLACK, COLOR_GREEN);
    init_pair(CP_GRENADE, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CP_PROJECTILE, COLOR_RED, COLOR_BLUE);

    // Imposta colori di sfondo per le finestre
    wbkgd(score, COLOR_PAIR(CP_SCORE_BG));
    wbkgd(info, COLOR_PAIR(CP_INFO));
    wbkgd(campo, COLOR_PAIR(CP_FIELD_BG));

    refresh();
    getmaxyx(campo, max_y, max_x); // Ottiene dim della finestra di gioco

    bool play_again = false;
    // Ciclo per permettere di rigiocare dopo la fine della partita
    do {
        game_over = false;
        user_quit = false;
        livello_vinto = false;
        init_game();

        // Ciclo che gestisce le vite del giocatore (una partita intera)
        while (stato.vite > 0 && !livello_vinto && !user_quit) {
            manche_terminata = false;
            stato.tempo_rimanente = TEMPO_MANCHE;

            // Pulisce le finestre
            werase(campo);
            werase(info);
            box(campo, 0, 0);
            box(info, 0, 0);
            wrefresh(campo);
            wrefresh(info);

            init_buffer(&buffer);

            // Dichiarazione  thread per la manche
            pthread_t t_rana, t_timer, t_consumer, t_input;
            int total_crocs = NUM_CORSIE * NUM_COC;
            pthread_t coc_threads[total_crocs];

            srand(time(NULL)); // Inizializzazione del generatore di numeri casuali

            // Creazione dei thread per i coccodrilli
            for (int corsia = 0; corsia < NUM_CORSIE; corsia++) {
                // Calcola parametri casuali per la corsia (direzione, velocità)
                int direzione = (corsia % 2 == 0) ? 1 : -1;
                int velocita = (rand() % (200000 - 60000 + 1) + 60000);

                // Calcola la spaziatura tra i coccodrilli per evitare sovrapposizioni
                int total_lane_distance = 0;
                int croc_positions[NUM_COC];
                int croc_widths[NUM_COC];
                for (int j = 0; j < NUM_COC; j++) {
                    int larghezza_coc = (rand() % (LARGHEZZA_COC_MAX - LARGHEZZA_COC_MIN + 1)) + LARGHEZZA_COC_MIN;
                    croc_widths[j] = larghezza_coc;
                    
                    croc_positions[j] = total_lane_distance;
                    
                    int random_spacing = larghezza_coc + (rand() % (DISTANZA_MAX_COC - DISTANZA_MIN_COC + 1) + DISTANZA_MIN_COC);
                    total_lane_distance += random_spacing;
                }

                int distanza_loop = total_lane_distance;


                // Crea i thread per ogni coccodrillo nella corsia
                for (int j = 0; j < NUM_COC; j++) {
                    int start_x;
                    if (direzione > 0) { 
                        start_x = -croc_widths[j] - croc_positions[j];
                    } else { 
                        start_x = max_x + croc_positions[j];
                    }

                    CocArgs *args = malloc(sizeof(CocArgs));
                    if (!args) { perror("malloc"); exit(1); }
                    
                    args->corsia = corsia;
                    args->start_x = start_x;
                    args->direzione = direzione;
                    args->velocita = velocita;
                    args->larghezza = croc_widths[j];
                    args->distanza_loop = distanza_loop;
                    args->id = (corsia * NUM_COC) + j + 1;

                    int thread_index = corsia * NUM_COC + j;
                    pthread_create(&coc_threads[thread_index], NULL, croc_thread, (void*)args);
                }
            }

            // Creazione dei thread principali del gioco
            pthread_create(&t_input, NULL, input_thread, NULL);
            pthread_create(&t_rana, NULL, rana_thread, NULL);
            pthread_create(&t_timer, NULL, game_timer, NULL);
            pthread_create(&t_consumer, NULL, consumer_thread, NULL);

            // Il main aspetta la fine della manche
            while (!manche_terminata && !livello_vinto && !user_quit) {
                usleep(100000);
            }
            
            if (user_quit) {
                game_over = true;
            }

            sem_post(&buffer.full); // Sblocca il consumer se è in attesa

            // Invia una richiesta di cancellazione a tutti i thread attivi
            pthread_cancel(t_input);
            pthread_cancel(t_rana);
            pthread_cancel(t_timer);
            for (int i = 0; i < total_crocs; i++) {
                pthread_cancel(coc_threads[i]);
            }

            // Attende la terminazione effettiva di ogni thread
            pthread_join(t_input, NULL);
            pthread_join(t_rana, NULL);
            pthread_join(t_timer, NULL);
            for (int i = 0; i < total_crocs; i++) {
                pthread_join(coc_threads[i], NULL);
            }
            pthread_join(t_consumer, NULL);

            cleanup_buffer(&buffer); // Pulisce le risorse del buffer
            
            // Decrementa una vita se la manche è finita per morte
            if (manche_terminata) {
                stato.vite--;
            }

        }
        // Mostra la schermata di fine partita
        bool vittoria_gioco = (livello_vinto && stato.vite > 0);
        play_again = draw_fine(vittoria_gioco, user_quit);

    } while (play_again);

    // Cleanup finale prima di uscire
    delwin(score);
    delwin(campo);
    delwin(info);
    endwin();

    return 0;
}
