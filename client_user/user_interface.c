#include <stdio.h>
#include <string.h>
#include "../include/user_interface.h"

void print_menu() {
    printf("\n===== CLIENT USER =====\n");
    printf("1. Trimite comandă (cut)\n");
    printf("2. Extrage audio (REST)\n");
    printf("3. Exit\n");
    printf("=======================\n");
    printf("Alege opțiunea: ");
}

void get_user_input(char *input, size_t size, const char *prompt) {
    printf("%s", prompt);
    fgets(input, size, stdin);
    input[strcspn(input, "\n")] = 0; // Eliminăm newline-ul
}