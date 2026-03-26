# 🐸 Frogger Clone per Linux (Progetto SO)

*Leggi in altre lingue: [English](README.md)*

[![C](https://img.shields.io/badge/C-Standard%20C99%2F11-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Linux-UNIX-yellow.svg)](https://www.linux.org/)
[![POSIX](https://img.shields.io/badge/POSIX-Threads%20%26%20IPC-green.svg)](https://en.wikipedia.org/wiki/POSIX)

Questo repository contiene il codice sorgente per una versione modificata del classico gioco **"Frogger"**, sviluppata come progetto finale per il corso di Sistemi Operativi. 

Il progetto esplora i concetti fondamentali della programmazione concorrente e di sistema in ambiente UNIX/Linux, focalizzandosi in particolar modo sulla gestione delle risorse a basso livello, sui **Processi** e sui **Thread**. Ogni elemento di gioco a schermo (la rana, le auto, i tronchi, ecc.) è gestito come un'entità completamente indipendente.

## 📂 Struttura del Repository e Architettura

Il progetto è diviso in due implementazioni distinte per dimostrare diversi paradigmi di programmazione concorrente:

### 1. 📁 `Processi/` (Implementazione Multi-Processo)
* Si basa su processi creati tramite `fork()`.
* **Comunicazione Inter-Processo (IPC):** La sincronizzazione dei dati, lo stato del gioco e le coordinate delle entità vengono scambiate utilizzando le **Pipe**.
* Gestisce a basso livello il movimento, il rilevamento delle collisioni e le transizioni di livello interamente tramite la comunicazione tra processi indipendenti.

### 2. 📁 `Thread/` (Implementazione Multi-Threading)
* Basata sull'utilizzo dei thread POSIX (`pthreads`).
* **Sincronizzazione & IPC:** La gestione della memoria condivisa e la comunicazione tra i thread avvengono tramite mutex, variabili di condizione e **Buffer Circolari**.

## 🛠️ Tecnologie Utilizzate
* **Linguaggio:** C (Standard C99/C11)
* **Sistema di compilazione:** `make` e `gcc`
* **Ambiente:** Sistemi operativi UNIX/Linux (o WSL su Windows)
* **Librerie di sistema:** `<unistd.h>`, `<pthread.h>`, `<sys/wait.h>`, `<sys/ipc.h>`, `ncurses` (per il rendering su terminale).

## 🚀 Come Compilare ed Eseguire

Per poter compilare ed eseguire il gioco, assicurati di avere installato un compilatore C (`gcc`) e lo strumento `make` sul tuo sistema.

1. Clona il repository:
   ```bash
   git clone [https://github.com/Mastrone/ProgettoSO.git](https://github.com/Mastrone/ProgettoSO.git)
   cd ProgettoSO
2. Naviga nella cartella dell'implementazione che desideri testare (es. Processi):
   ```bash
    cd Processi
    (oppure cd Thread/Thread per la versione multi-thread)
3. Compila il codice utilizzando il Makefile fornito:
   ```bash
    make
4. Esegui il file binario generato (generalmente chiamato frogger o main a seconda della configurazione del Makefile):
   ```bash
    ./nome_eseguibile
6. Per pulire i file oggetto e gli eseguibili generati:
   ```bash
    make clean
Autore

Alessandro Mastrone

[![LinkedIn](https://img.shields.io/badge/LinkedIn-blue?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alessandro-mastrone-a59531187/)    [![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Mastrone)

Progetto realizzato per scopi didattici nell'ambito del corso di Sistemi Operativi.
