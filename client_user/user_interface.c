#include <stdio.h>
#include <string.h>
#include "../include/user_interface.h"

#include <stdio.h>
#include <string.h>
#include "../include/user_interface.h"

void clearScreen() {
    // Trimite codul ANSI pentru a curata ecranul si a muta cursorul
    printf("\033[2J\033[1;1H");
}

void print_main_menu() {
    printf("\n===== CLIENT USER =====\n");
    printf("1. Editare video\n");
    printf("2. Exit\n");
    printf("=======================\n");
    printf("Alege opțiunea: ");
    fflush(stdout); // Asigura afisarea prompt-ului imediat
}

void print_edit_menu() {
    printf("\n--- Editări disponibile ---\n");
    printf("1. Cut (tăiere secvență)\n");
    printf("2. Extrage audio\n");
    printf("3. Concatenare\n");
    printf("4. Schimbă rezoluție\n");
    printf("5. Cut Except\n");
    printf("6. Speed Segment\n");
    printf("0. Finalizează editările și salvează\n");
    printf("----------------------------\n");
    printf("Alege o acțiune: ");
    fflush(stdout); // Asigura afisarea prompt-ului imediat
}

void get_user_input(char *buffer, size_t size, const char *prompt) {
    printf("%s", prompt);
    fflush(stdout);
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Elimina newline de la final
}
