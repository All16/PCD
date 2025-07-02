#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h> // Pentru INET_ADDRSTRLEN
#include "../include/client_tracker.h"

// Definim variabilele globale aici pentru a avea un exemplu complet
ClientInfo client_list[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(const char *ip, int sock_fd) {
    pthread_mutex_lock(&client_mutex);
    if (client_count < MAX_CLIENTS) {
        strncpy(client_list[client_count].ip, ip, INET_ADDRSTRLEN);
        client_list[client_count].socket_fd = sock_fd;
        client_count++;
    }
    pthread_mutex_unlock(&client_mutex);
}

void remove_client_by_ip(const char *ip) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (strcmp(client_list[i].ip, ip) == 0) {

            // ### START MODIFICARE: Trimite notificare clientului ###
            const char* kick_msg = "SERVER: Ai fost deconectat de catre un administrator.\n";
            write(client_list[i].socket_fd, kick_msg, strlen(kick_msg));
            // Asteptam putin pentru a da timp mesajului sa fie trimis
            usleep(3000);

            close(client_list[i].socket_fd);

            // Mutam restul elementelor pentru a umple golul
            for (int j = i; j < client_count - 1; ++j) {
                client_list[j] = client_list[j + 1];
            }
            client_count--;
            printf("[TRACKER] Client %s eliminat. Ramasi: %d\n", ip, client_count);
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void get_client_list(char *dest_buffer, size_t max_len) {
    pthread_mutex_lock(&client_mutex);
    snprintf(dest_buffer, max_len, "Utilizatori activi:\n");
    for (int i = 0; i < client_count; ++i) {
        strncat(dest_buffer, " - ", max_len - strlen(dest_buffer) - 1);
        strncat(dest_buffer, client_list[i].ip, max_len - strlen(dest_buffer) - 1);
        strncat(dest_buffer, "\n", max_len - strlen(dest_buffer) - 1);
    }
    pthread_mutex_unlock(&client_mutex);
}