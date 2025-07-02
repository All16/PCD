// === client_admin/admin_interface.c ===
#include <stdio.h>
#include "../include/admin_interface.h"

void clearScreen() {
    printf("\033[2J\033[1;1H");
}
void printMenu() {
    clearScreen();
    printf("\n==== MENIU ADMIN ====\n");
    printf("1. Lista clienti activi\n");
    printf("2. Lista joburi active\n");
    printf("3. Statistici server\n");
    printf("4. Exit\n");
    printf("5. Deconecteaza un client\n");
    printf("=====================\n");
    printf("Alege optiunea: ");
}

