#include "utils.h"

// Inizializza la struttura del buffer condiviso
void init_buffer(GameObjectBuffer *buffer) {
    buffer->in = 0;  // Indice di inserimento
    buffer->out = 0; // Indice di estrazione
    pthread_mutex_init(&buffer->mutex, NULL); // Inizializza il mutex per l'accesso esclusivo
    // Il semaforo empty conta gli slot vuoti - parte da BUFFER_SIZE
    sem_init(&buffer->empty, 0, DIMENSIONE_BUFFER);
    // Il semaforo full conta gli slot pieni - parte da 0
    sem_init(&buffer->full, 0, 0);
}

// Aggiunge un oggetto al buffer (funzione del produttore)
void produce(GameObjectBuffer *buffer, GameObject obj) {
    // Attende che ci sia uno slot vuoto (decrementa empty)
    sem_wait(&buffer->empty);
    // Blocca l'accesso al buffer
    pthread_mutex_lock(&buffer->mutex);

    // Inserisce l'oggetto e avanza l'indice 'in' in modo circolare
    buffer->buffer[buffer->in] = obj;
    buffer->in = (buffer->in + 1) % DIMENSIONE_BUFFER;

    // Sblocca l'accesso al buffer
    pthread_mutex_unlock(&buffer->mutex);
    // Segnala che c'è un nuovo slot pieno (incrementa full)
    sem_post(&buffer->full);
}

// Estrae un oggetto dal buffer (funzione del consumatore)
GameObject consume(GameObjectBuffer *buffer) {
    GameObject obj;
    // Attende che ci sia uno slot pieno (decrementa full)
    sem_wait(&buffer->full);
    // Blocca l'accesso al buffer
    pthread_mutex_lock(&buffer->mutex);

    // Estrae l'oggetto e aumenta l'indice in modo circolare
    obj = buffer->buffer[buffer->out];
    buffer->out = (buffer->out + 1) % DIMENSIONE_BUFFER;

    // Sblocca l'accesso al buffer
    pthread_mutex_unlock(&buffer->mutex);
    // Segnala che c'è un nuovo slot vuoto (incrementa empty)
    sem_post(&buffer->empty);
    return obj;
}

// Libera le risorse del buffer (mutex e semafori)
void cleanup_buffer(GameObjectBuffer *buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
}

// Calcola in quale corsia del fiume si trova una data coordinata Y
int calc_corsia(int y, int max_y, int altezza_sponda, int num_corsie) {
    // Calcola l'altezza di una singola corsia
    int altezza_corsia = (max_y - 2 * altezza_sponda) / num_corsie;
    // Determina l'indice della corsia in base alla posizione Y
    int corsia = (y - altezza_sponda) / altezza_corsia;
    
    // Controllo per assicurarsi che il valore sia valido
    if (corsia < 0) {
        corsia = 0;
    }
    if (corsia >= num_corsie) {
        corsia = num_corsie - 1;
    }
    return corsia;
}