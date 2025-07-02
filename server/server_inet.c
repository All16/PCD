#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include "../include/client_tracker.h"

#define PORT 5001

extern volatile sig_atomic_t running;

typedef struct {
    int socket_fd;
    char ip_str[INET_ADDRSTRLEN];
} thread_client_info_t;

void* handle_client_connection(void* arg) {
    thread_client_info_t* client_info = (thread_client_info_t*)arg;
    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(client_info->socket_fd, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';
        printf("[INET-%s] Comandă: %s", client_info->ip_str, buffer);
        write(client_info->socket_fd, "[Server] OK\n", 13);
    }

    printf("[INET] Clientul %s s-a deconectat.\n", client_info->ip_str);
    remove_client_by_ip(client_info->ip_str);
    free(client_info);
    pthread_exit(NULL);
}

void* handle_user_clients(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Setam socket-ul non-blocant pentru a putea folosi select()
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[INET] bind failed");
        pthread_exit(NULL);
    }

    listen(server_fd, 10);
    printf("[INET] Server INET user ascultă pe port %d\n", PORT);

    fd_set fds;
    struct timeval timeout;

    while (running) {
        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        timeout.tv_sec = 1; // Timeout de 1 secunda
        timeout.tv_usec = 0;

        // Asteptam non-blocant pentru conexiuni noi
        int ready = select(server_fd + 1, &fds, NULL, NULL, &timeout);

        if (ready > 0 && FD_ISSET(server_fd, &fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

            if (client_fd >= 0) {
                thread_client_info_t* client_info = malloc(sizeof(thread_client_info_t));
                client_info->socket_fd = client_fd;
                inet_ntop(AF_INET, &client_addr.sin_addr, client_info->ip_str, sizeof(client_info->ip_str));

                printf("[INET] Conectat client: %s. Pornire thread dedicat.\n", client_info->ip_str);
                add_client(client_info->ip_str, client_info->socket_fd);

                pthread_t client_thread;
                if (pthread_create(&client_thread, NULL, handle_client_connection, client_info) != 0) {
                    perror("pthread_create failed");
                    free(client_info);
                    close(client_fd);
                }
                pthread_detach(client_thread);
            }
        }
        // Daca ready == 0 (timeout) sau ready < 0 (eroare), bucla se reia si verifica 'running'
    }

    close(server_fd);
    printf("[INET] Fir INET oprit curat.\n");
    pthread_exit(NULL);
}