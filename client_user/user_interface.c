#include <stdio.h>
#include <string.h>
#include "../include/user_interface.h"

void print_main_menu() {
    printf("\n===== CLIENT USER =====\n");
    printf("1. Editare video\n");
    printf("2. Exit\n");
    printf("=======================\n");
    printf("Alege opțiunea: ");
}

void print_edit_menu() {
    printf("\n--- Editări disponibile ---\n");
    printf("1. Cut (tăiere secvență)\n");
    printf("2. Extrage audio\n");
    printf("3. Concatenare\n");
    printf("4. Schimbă rezoluție\n");
    printf("0. Finalizează editările\n");
    printf("----------------------------\n");
    printf("Alege o acțiune: ");
}

void get_user_input(char *input, size_t size, const char *prompt) {
    printf("%s", prompt);
    fgets(input, size, stdin);
    input[strcspn(input, "\n")] = 0;
}
