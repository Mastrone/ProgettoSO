# 🐸 Frogger Clone for Linux (OS Project)

*Read this in other languages: [Italiano](README.it.md)*

[![C](https://img.shields.io/badge/C-Standard%20C99%2F11-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Linux-UNIX-yellow.svg)](https://www.linux.org/)
[![POSIX](https://img.shields.io/badge/POSIX-Threads%20%26%20IPC-green.svg)](https://en.wikipedia.org/wiki/POSIX)

This repository contains the source code for a modified version of the classic **"Frogger"** game, developed as the final project for the Operating Systems course. 

The project explores the fundamental concepts of concurrent and system programming in a UNIX/Linux environment, focusing specifically on low-level resource management, **Processes**, and **Threads**. Every on-screen game element (the frog, cars, logs, etc.) is managed as a completely independent entity.

## 📂 Repository Structure & Architecture

The project is divided into two distinct implementations to demonstrate different concurrent programming paradigms:

### 1. 📁 `Processi/` (Multi-Process Implementation)
* Relies on processes created via `fork()`.
* **Inter-Process Communication (IPC):** Data synchronization, game state, and entity coordinates are exchanged using **Pipes**.
* Handles low-level movement, collision detection, and level transitions entirely through independent process communication.

### 2. 📁 `Thread/` (Multi-Threading Implementation)
* Based on POSIX threads (`pthreads`).
* **Synchronization & IPC:** Shared memory management and communication between threads are handled via mutexes, condition variables, and **Circular Buffers**.

## 🛠️ Tech Stack
* **Language:** C (Standard C99/C11)
* **Build System:** `make` and `gcc`
* **Environment:** UNIX/Linux Operating Systems (or WSL on Windows)
* **System Libraries:** `<unistd.h>`, `<pthread.h>`, `<sys/wait.h>`, `<sys/ipc.h>`, `ncurses` (for terminal rendering).

## 🚀 How to Build and Run

To compile and run the game, ensure you have a C compiler (`gcc`) and `make` installed on your system.

1. Clone the repository:
   ```bash
   git clone https://github.com/Mastrone/ProgettoSO.git
   cd ProgettoSO

2. Navigate to your preferred implementation folder (e.g., Processes):
   ```bash
    cd Processi
    (or cd Thread/Thread for the multi-threaded version).

3. Compile the code using the provided Makefile:
   ```bash
    make
4. Run the generated executable (usually named frogger or main depending on the Makefile configuration):
   ```bash
    ./nome_eseguibile
5. To clean up object files and executables:
   ```bash
    make clean
Author

Alessandro Mastrone

[![LinkedIn](https://img.shields.io/badge/LinkedIn-blue?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alessandro-mastrone-a59531187/)    [![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Mastrone)

Developed for educational purposes within the Operating Systems course.
