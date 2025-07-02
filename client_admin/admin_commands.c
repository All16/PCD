// === client_admin/admin_commands.c ===
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/admin_commands.h"
#include "../include/admin_interface.h"

#define BUFFER_SIZE 256

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

int authenticate(const char *username, const char *password) {
    return strcmp(username, "admin") == 0 && strcmp(password, "password") == 0;
}

int handleLogin() {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    clearScreen();

    printf("[ADMIN] Enter username: \n");
    scanf("%s", username);
    printf("[ADMIN] Enter password: \n");
    scanf("%s", password);

    if (authenticate(username, password)) {
        printf("[ADMIN] Login successful!\n");
        return 1;
    } else {
        printf("[ADMIN] Invalid credentials.\n");
        handleLogin();
        return 0;
    }
}

int process_command(int sock_fd, int option) {
    char cmd[BUFFER_SIZE] = {0};

    if (option == 5) {  // KICK client
        char ip[64];
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
                printf("[ADMIN] Optiune invalida \n");
                return -1;
        }
    }

    if (write(sock_fd, cmd, strlen(cmd)) <= 0 || write(sock_fd, "\n", 1) <= 0) {
        perror("[ADMIN] Eroare la trimiterea comenzii \n");
        return -1;
    }

    char buffer[BUFFER_SIZE] = {0};
    if (read(sock_fd, buffer, sizeof(buffer) - 1) <= 0) {
        perror("[ADMIN] Eroare la citirea raspunsului \n");
        return -1;
    }

    printf("[SERVER] %s", buffer);
    return 0;
}

