#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/admin_commands.h"
#include "../include/admin_interface.h"

#define BUFFER_SIZE 256
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

// Functia read_full_response trebuie declarata aici daca e folosita
// sau inclusa dintr-un header comun.
void read_full_response(int sock_fd);

int authenticate(const char *username, const char *password) {
    return strcmp(username, "admin") == 0 && strcmp(password, "password") == 0;
}

int handleLogin() {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    clearScreen();
    printf("[ADMIN] Enter username: ");
    scanf("%s", username);
    printf("[ADMIN] Enter password: ");
    scanf("%s", password);
    if (authenticate(username, password)) {
        printf("[ADMIN] Login successful!\n");
        usleep(1000000);
        return 1;
    } else {
        printf("[ADMIN] Invalid credentials.\n");
        usleep(1000000);
        return handleLogin();
    }
}

int process_command(int sock_fd, int option) {
    char cmd[BUFFER_SIZE] = {0};

    // ### START MODIFICARE: Logica pentru KICK ###
    if (option == 5) {
        // Pas 1: Trimitem comanda LIST pentru a afisa clientii
        const char* list_cmd = "LIST\n";
        if (write(sock_fd, list_cmd, strlen(list_cmd)) <= 0) {
            perror("[ADMIN] Eroare la trimiterea comenzii LIST");
            return -1;
        }

        // Asteptam si citim raspunsul
        usleep(100000);
        clearScreen();
        printf("--- Clienti Conectati ---\n");
        read_full_response(sock_fd);
        printf("-------------------------\n");

        // Pas 2: Cerem IP-ul de la admin
        printf("Introduceti IP-ul clientului de deconectat (sau 'anulare' pentru a renunta): ");
        char ip[64];
        scanf("%s", ip);
        getchar(); // Consuma newline

        if (strcmp(ip, "anulare") == 0) {
            return 0; // Se intoarce la meniul principal
        }

        // Pas 3: Construim si trimitem comanda KICK
        snprintf(cmd, sizeof(cmd), "KICK %s", ip);

    }
        // ### END MODIFICARE ###
    else {
        switch (option) {
            case 1: strncpy(cmd, "LIST", sizeof(cmd) - 1); break;
            case 2: strncpy(cmd, "JOBS", sizeof(cmd) - 1); break;
            case 3: strncpy(cmd, "STATS", sizeof(cmd) - 1); break;
            case 4: strncpy(cmd, "exit", sizeof(cmd) - 1); break;
            default:
                printf("[ADMIN] Optiune invalida \n");
                return 0; // Nu e eroare fatala
        }
    }

    // Trimitem comanda finala (LIST, KICK, etc.) catre server
    if (write(sock_fd, cmd, strlen(cmd)) <= 0 || write(sock_fd, "\n", 1) <= 0) {
        perror("[ADMIN] Eroare la trimiterea comenzii");
        return -1;
    }

    return 0;
}