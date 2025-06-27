#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stddef.h>

void print_menu();
void get_user_input(char *input, size_t size, const char *prompt);

#endif