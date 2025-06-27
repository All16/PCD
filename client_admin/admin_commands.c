#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/admin_commands.h"

#define MAX_COMMAND_LENGTH 256
#define BUFFER_SIZE 256

void execute_command(const char *command) {
    if (strcmp(command, "add_user") == 0) {
        printf("Executing: Add User\n");
        // Add user logic here
    } else if (strcmp(command, "remove_user") == 0) {
        printf("Executing: Remove User\n");
        // Remove user logic here
    } else if (strcmp(command, "list_users") == 0) {
        printf("Executing: List Users\n");
        // List users logic here
    } else {
        printf("Unknown command: %s\n", command);
    }
}

void print_help() {
    printf("Available commands:\n");
    printf("  add_user     - Add a new user\n");
    printf("  remove_user  - Remove an existing user\n");
    printf("  list_users   - List all users\n");
    printf("  help         - Show this help message\n");
    printf("  exit         - Exit the program\n");
}

int process_command(int sock_fd, int option) {
    char cmd[BUFFER_SIZE] = {0};

    switch (option) {
        case 1:
            strncpy(cmd, "LIST", sizeof(cmd) - 1);
            break;
        case 2:
            strncpy(cmd, "JOBS", sizeof(cmd) - 1);
            break;
        case 3:
            strncpy(cmd, "STATS", sizeof(cmd) - 1);
            break;
        case 4:
            strncpy(cmd, "exit", sizeof(cmd) - 1);
            break;
        default:
            printf("Opțiune invalidă\n");
            return -1;
    }

    // Trimitem comanda
    if (write(sock_fd, cmd, strlen(cmd)) <= 0) {
        perror("[ADMIN CLIENT] Eroare la trimiterea comenzii");
        return -1;
    }

    // Adăugăm newline
    if (write(sock_fd, "\n", 1) <= 0) {
        perror("[ADMIN CLIENT] Eroare la trimiterea newline-ului");
        return -1;
    }

    // Citim și afișăm răspunsul
    char buffer[BUFFER_SIZE] = {0};
    if (read(sock_fd, buffer, sizeof(buffer) - 1) <= 0) {
        perror("[ADMIN CLIENT] Eroare la citirea răspunsului");
        return -1;
    }
    printf("[SERVER] %s", buffer);

    return 0;
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    printf("Admin Command Interface\n");
    printf("Type 'help' for a list of commands.\n");

    while (1) {
        printf("\n> ");
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            printf("Error reading input. Exiting.\n");
            break;
        }

        // Remove trailing newline character
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            printf("Exiting program.\n");
            break;
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else {
            execute_command(command);
        }
    }

    return 0;
}