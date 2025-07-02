// === server/main.c (sau cum se numeste fisierul tau principal) ===

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include "../include/job_queue.h"

// Declaratii pentru functiile din celelalte fisiere
void* handle_user_clients(void* arg);
void* handle_admin_socket(void* arg);
void* start_api_server_thread(void* arg);
void* worker_thread_func(void* arg);

// Variabila globala pentru a opri toate thread-urile elegant
extern volatile sig_atomic_t running;

void handle_signal(int signal) {
    printf("\n[SYSTEM] Semnal de oprire primit. Inchidere server...\n");
    running = 0;
    // Deblocheaza worker-ul daca asteapta un job
    job_queue_destroy(); // Functia ta ar trebui sa faca broadcast/signal
}

int main() {
    signal(SIGINT, handle_signal);

    // Initializam coada de job-uri
    job_queue_init();

    pthread_t user_tid, admin_tid, api_tid, worker_tid;

    printf("[SYSTEM] Pornire thread-uri server...\n");

    // Pornim thread-urile existente
    pthread_create(&user_tid, NULL, handle_user_clients, NULL);
    pthread_create(&admin_tid, NULL, handle_admin_socket, NULL);

    // Pornim noile thread-uri pentru API si Worker
    pthread_create(&api_tid, NULL, start_api_server_thread, NULL);
    pthread_create(&worker_tid, NULL, worker_thread_func, NULL);

    // Asteptam terminarea thread-urilor
    pthread_join(user_tid, NULL);
    pthread_join(admin_tid, NULL);
    pthread_join(api_tid, NULL);
    pthread_join(worker_tid, NULL);

    printf("[SYSTEM] Serverul s-a oprit complet.\n");
    return 0;
}