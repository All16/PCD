#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/command_sender.h"

int send_command(int sock_fd, const char *command, char *response, size_t response_size) {
    // Trimitem comanda către server
    if (write(sock_fd, command, strlen(command)) <= 0) {
        perror("[CLIENT] Eroare la trimiterea comenzii");
        return -1;
    }

    // Adăugăm newline pentru a marca sfârșitul comenzii
    if (write(sock_fd, "\n", 1) <= 0) {
        perror("[CLIENT] Eroare la trimiterea newline-ului");
        return -1;
    }

    // Citim răspunsul de la server
    memset(response, 0, response_size);
    if (read(sock_fd, response, response_size - 1) <= 0) {
        perror("[CLIENT] Eroare la citirea răspunsului");
        return -1;
    }

    return 0;
}