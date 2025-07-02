#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <microhttpd.h>
#include "../include/common.h"
#include "../include/job_queue.h"
#include "../include/rest_api.h"

extern volatile sig_atomic_t running;

pthread_t thread_admin, thread_users, thread_jobs, thread_rest_json;
int admin_socket_fd = -1;
int user_socket_fd = -1;

void cleanup() {
    printf("[INFO] Închidere server...\n");

    running = 0;
    pthread_cond_broadcast(&not_empty);  // deblochează job_processor dacă așteaptă

    pthread_join(thread_admin, NULL);
    pthread_join(thread_users, NULL);
    pthread_join(thread_jobs, NULL);
    pthread_join(thread_rest_json, NULL);

    if (admin_socket_fd != -1) close(admin_socket_fd);
    if (user_socket_fd != -1) close(user_socket_fd);

    job_queue_destroy();
}

void sigint_handler(int sig) {
    printf("\n[CTRL+C] Semnal de închidere primit.\n");
    running = 0;
}

extern void* handle_admin_socket(void*);
extern void* handle_user_clients(void*);
extern void* job_processor(void*);
extern void* start_rest_server(void*);

int main() {
    signal(SIGINT, sigint_handler);
    printf("[START] Pornim serverul V-Edit...\n");

    job_queue_init();

    if (pthread_create(&thread_admin, NULL, handle_admin_socket, NULL) != 0) {
        perror("Eroare creare fir admin");
        cleanup();
        exit(1);
    }

    if (pthread_create(&thread_users, NULL, handle_user_clients, NULL) != 0) {
        perror("Eroare creare fir user");
        cleanup();
        exit(1);
    }

    if (pthread_create(&thread_jobs, NULL, job_processor, NULL) != 0) {
        perror("Eroare creare fir job");
        cleanup();
        exit(1);
    }

    if (pthread_create(&thread_rest_json, NULL, start_rest_server, NULL) != 0) {
        perror("Eroare creare fir REST");
        cleanup();
        exit(1);
    }

    struct MHD_Daemon* rest_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 5000,
                                                      NULL, NULL, &handle_request, NULL,
                                                      MHD_OPTION_END);
    if (!rest_daemon) {
        fprintf(stderr, "❌ Eroare: MHD_start_daemon a eșuat!\n");
        cleanup();
        return 1;
    }

    printf("[REST] Server HTTP REST activ pe port 5000.\n");

    // Așteaptă până se apasă Ctrl+C
    while (running) {
        sleep(1);
    }

    MHD_stop_daemon(rest_daemon);
    cleanup();
    return 0;
}
