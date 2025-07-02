// === include/admin_commands.h ===
#ifndef ADMIN_COMMANDS_H
#define ADMIN_COMMANDS_H

int authenticate(const char *username, const char *password);
int handleLogin();
int process_command(int sock_fd, int option);

#endif