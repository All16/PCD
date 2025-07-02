#include <stdio.h>
#include <stdlib.h> // Adaugat pentru exit()
#include <pthread.h>
#include <signal.h>
#include <unistd.h> // Pentru usleep
#include "../include/job_queue.h"

// Declaratii pentru functiile din celelalte fisiere
void* handle_user_clients(void* arg);
void* handle_admin_socket(void* arg);
void* start_api_server_thread(void* arg);
void* worker_thread_func(void* arg);

// Variabila globala pentru a opri toate thread-urile elegant
extern volatile sig_atomic_t running;

void handle_signal(int signal) {
    if (running == 0) {
        // Fortam iesirea daca se apasa Ctrl+C de mai multe ori
        fprintf(stderr, "\nOprire fortata!\n");
        exit(1);
    }
    printf("\n[SYSTEM] Semnal de oprire primit. Inchidere server...\n");
    running = 0;
    // Deblocheaza worker-ul daca asteapta un job
    job_queue_destroy();
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    job_queue_init();

    pthread_t user_tid, admin_tid, api_tid, worker_tid;

    printf("[SYSTEM] Pornire thread-uri server...\n");

    pthread_create(&user_tid, NULL, handle_user_clients, NULL);
    pthread_create(&admin_tid, NULL, handle_admin_socket, NULL);
    pthread_create(&api_tid, NULL, start_api_server_thread, NULL);
    pthread_create(&worker_tid, NULL, worker_thread_func, NULL);

    // Asteptam terminarea thread-urilor
    printf("[SYSTEM] Serverul ruleaza. Apasa Ctrl+C pentru a opri.\n");
    pthread_join(user_tid, NULL);
    printf("[SYSTEM] Thread-ul INET s-a oprit.\n");

    pthread_join(admin_tid, NULL);
    printf("[SYSTEM] Thread-ul UNIX s-a oprit.\n");

    pthread_join(api_tid, NULL);
    printf("[SYSTEM] Thread-ul API s-a oprit.\n");

    pthread_join(worker_tid, NULL);
    printf("[SYSTEM] Thread-ul Worker s-a oprit.\n");

    printf("[SYSTEM] Serverul s-a oprit complet.\n");
    return 0;
}
