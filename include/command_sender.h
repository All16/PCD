#ifndef COMMAND_SENDER_H
#define COMMAND_SENDER_H

#include <stddef.h>

int send_command(int sock_fd, const char *command, char *response, size_t response_size);

#endif