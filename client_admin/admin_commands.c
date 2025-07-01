// === client_admin/admin_commands.c ===
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/admin_commands.h"

#define BUFFER_SIZE 256

int process_command(int sock_fd, int option) {
    char cmd[BUFFER_SIZE] = {0};

    if (option == 5) {  // KICK client
        char ip[64];
        printf("Introdu IP-ul clientului de deconectat: ");
        scanf("%s", ip);
        getchar();
        snprintf(cmd, sizeof(cmd), "KICK %s", ip);
    } else {
        switch (option) {
            case 1: strncpy(cmd, "LIST", sizeof(cmd) - 1); break;
            case 2: strncpy(cmd, "JOBS", sizeof(cmd) - 1); break;
            case 3: strncpy(cmd, "STATS", sizeof(cmd) - 1); break;
            case 4: strncpy(cmd, "exit", sizeof(cmd) - 1); break;
            default:
                printf("Optiune invalida");
                return -1;
        }
    }

    if (write(sock_fd, cmd, strlen(cmd)) <= 0 || write(sock_fd, "\n", 1) <= 0) {
        perror("[ADMIN CLIENT] Eroare la trimiterea comenzii");
        return -1;
    }

    char buffer[BUFFER_SIZE] = {0};
    if (read(sock_fd, buffer, sizeof(buffer) - 1) <= 0) {
        perror("[ADMIN CLIENT] Eroare la citirea raspunsului");
        return -1;
    }

    printf("[SERVER] %s", buffer);
    return 0;
}

void print_menu() {
    printf("\n==== MENIU ADMIN ====\n");
    printf("1. Lista clienti activi\n");
    printf("2. Lista joburi active\n");
    printf("3. Statistici server\n");
    printf("4. Exit\n");
    printf("5. Deconecteaza un client\n");
    printf("=====================\n");
    printf("Alege optiunea: ");
}