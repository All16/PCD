// === server/server_inet.c ===
#include <stdio.h>
#include <stdlib.h> // Necesar pentru malloc/free
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

// O structura pentru a trimite datele clientului catre thread
typedef struct {
    int socket_fd;
    char ip_str[INET_ADDRSTRLEN];
} thread_client_info_t;

/**
 * @brief Functia care ruleaza in fiecare thread si gestioneaza un singur client.
 */
void* handle_client_connection(void* arg) {
    thread_client_info_t* client_info = (thread_client_info_t*)arg;
    char buffer[1024];
    ssize_t bytes_read;

    // Bucla de comunicare cu clientul. Se termina cand read() returneaza 0 sau -1.
    while ((bytes_read = read(client_info->socket_fd, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';
        printf("[INET-%s] Comandă: %s", client_info->ip_str, buffer);
        // Aici procesezi comanda si trimiti un raspuns
        write(client_info->socket_fd, "[Server] OK\n", 13);
    }

    // Daca am iesit din bucla, inseamna ca clientul s-a deconectat.
    printf("[INET] Clientul %s s-a deconectat.\n", client_info->ip_str);
    remove_client_by_ip(client_info->ip_str); // Apelam functia de stergere

    // Resursele sunt eliberate de functia remove_client_by_ip (care face close)
    // Eliberam memoria alocata pentru argumentul thread-ului
    free(client_info);
    pthread_exit(NULL);
}


/**
 * @brief Functia principala a serverului INET. Acum doar accepta conexiuni
 * si porneste un thread nou pentru fiecare.
 */
void* handle_user_clients(void* arg) {
    int server_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Optional: Permite reutilizarea adresei imediat
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        pthread_exit(NULL);
    }

    listen(server_fd, 10); // Am marit coada de asteptare

    printf("[INET] Server INET user ascultă pe port %d\n", PORT);

    while (running) {
        pthread_testcancel();

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // accept() este blocant, dar e ok, pentru ca asta e tot ce face acest thread.
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            if (running) perror("accept failed");
            continue; // Trecem la urmatoarea iteratie
        }

        // Alocam memorie pentru informatiile clientului, care vor fi trimise thread-ului
        thread_client_info_t* client_info = malloc(sizeof(thread_client_info_t));
        if (client_info == NULL) {
            perror("malloc for client_info failed");
            close(client_fd);
            continue;
        }

        // Populăm structura cu datele clientului
        client_info->socket_fd = client_fd;
        inet_ntop(AF_INET, &client_addr.sin_addr, client_info->ip_str, sizeof(client_info->ip_str));

        printf("[INET] Conectat client: %s. Pornire thread dedicat.\n", client_info->ip_str);
        add_client(client_info->ip_str, client_info->socket_fd);

        // Cream thread-ul care va gestiona acest client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client_connection, client_info) != 0) {
            perror("pthread_create failed");
            free(client_info);
            close(client_fd);
        }

        // Detasam thread-ul pentru a nu fi nevoiti sa ii dam join
        // Sistemul va elibera resursele la terminarea lui.
        pthread_detach(client_thread);
    }

    close(server_fd);
    printf("[INET] Fir INET oprit curat.\n");
    pthread_exit(NULL);
}