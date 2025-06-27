#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/admin_interface.h"

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

// Function prototypes
void displayMenu();
void handleLogin();
void handleAdminActions();
int authenticate(const char *username, const char *password);
void print_menu();
int get_user_option();

int main() {
    int choice;

    while (1) {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                handleLogin();
                break;
            case 2:
                printf("Exiting program. Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

void displayMenu() {
    printf("\n=== Admin Interface ===\n");
    printf("1. Login\n");
    printf("2. Exit\n");
}

void handleLogin() {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    if (authenticate(username, password)) {
        printf("Login successful!\n");
        handleAdminActions();
    } else {
        printf("Invalid username or password. Please try again.\n");
    }
}

void handleAdminActions() {
    printf("\n=== Admin Actions ===\n");
    printf("1. View logs\n");
    printf("2. Manage users\n");
    printf("3. Logout\n");
    // Add more admin actions as needed
}

int authenticate(const char *username, const char *password) {
    // Replace with actual authentication logic
    const char *adminUsername = "admin";
    const char *adminPassword = "password";

    return strcmp(username, adminUsername) == 0 && strcmp(password, adminPassword) == 0;
}

void print_menu() {
    printf("\n==== MENIU ADMIN ====\n");
    printf("1. Lista clienți activi\n");
    printf("2. Lista joburi active\n");
    printf("3. Statistici server\n");
    printf("4. Exit\n");
    printf("=====================\n");
    printf("Alege opțiunea: ");
}

int get_user_option() {
    int option;
    if (scanf("%d", &option) != 1) {
        while (getchar() != '\n'); // Consumăm intrarea invalidă
        return -1; // Opțiune invalidă
    }
    getchar(); // Consumăm '\n'
    return option;
}