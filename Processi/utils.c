#include "utils.h"

// Signal handler per il processo padre (main)
void handle_sigint_parent() {
    game_over = 1; // Imposta il flag per terminare i cicli di gioco
    cleanup_processes(); // Chiama la funzione di pulizia per i figli
    endwin(); // Ripristina il terminale
    exit(0);
}

// Signal handler per i processi figli
void handle_sigterm_child() {
    // Un processo figlio termina semplicemente quando riceve SIGTERM
    exit(0);
}

// Funzione per terminare tutti i processi figli creati
void cleanup_processes() {
    // Itera sull'array dei PID dei figli
    for (int i = 0; i < MAX_CHILD_PROCS; i++) {
        // Se il PID è valido (>0), invia il segnale di terminazione
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGTERM);
        }
    }
    // Attende la terminazione di tutti i figli per evitare processi zombie
    while (wait(NULL) > 0);
}

// Funzione per read che garantisce la lettura completa dei byte
ssize_t read_full(int fd, void *buf, size_t count) {
    size_t bytes_read = 0;
    // Continua a leggere finché non ha letto 'count' byte
    while (bytes_read < count) {
        ssize_t result = read(fd, (char*)buf + bytes_read, count - bytes_read);
        // Gestione degli errori
        if (result < 0) { 
            if (errno == EINTR) continue; // Se interrotto da un segnale, riprova
            return -1; // Errore di lettura
        }
        if (result == 0) break; // La pipe è stata chiusa
        bytes_read += result;
    }
    return bytes_read;
}

// Inizializza o resetta le variabili di stato del gioco
void init_game(void) {
    stato.punteggio = 0;
    stato.vite = VITE_INIZIALI;
    stato.tempo_rimanente = TEMPO_MANCHE;
    for (int i = 0; i < NUM_TANE; i++) stato.tane_occupate[i] = 0;
    // Calcola la posizione delle tane sullo schermo
    int start_x_tane = 6;
    for (int i = 0; i < NUM_TANE; i++) {
        posizioni_tane[i].x = start_x_tane + i * (2 * LARGHEZZA_RANA + 10);
        posizioni_tane[i].y = 1;
    }
}

// Calcola in quale corsia del fiume si trova una data coordinata Y
int calc_corsia(int y, int max_y, int altezza_sponda, int num_corsie) {
    // Calcola l'altezza di una singola corsia
    int corsia_altezza = (max_y - 2 * altezza_sponda) / num_corsie;
    // Determina l'indice della corsia
    int corsia = (y - altezza_sponda) / corsia_altezza;
    // Controlla che il valore sia dentro i limiti validi
    if (corsia < 0) corsia = 0;
    if (corsia >= num_corsie) corsia = num_corsie - 1;
    return corsia;
}