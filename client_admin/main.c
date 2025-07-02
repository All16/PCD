// === client_admin/main.c ===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../include/admin_interface.h"
#include "../include/admin_commands.h"

#define ADMIN_SOCKET_PATH "/tmp/vedit_admin_socket"

int main() {
    int sock_fd;
    struct sockaddr_un addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[ADMIN CLIENT] Eroare la creare socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ADMIN_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[ADMIN CLIENT] Eroare la conectare");
        close(sock_fd);
        return 1;
    }

    char buffer[256] = {0};
    if (read(sock_fd, buffer, sizeof(buffer) - 1) > 0) {
        printf("%s", buffer);
    }

    while (!handleLogin());

    int opt;
    while (1) {
        printMenu();
        if (scanf("%d", &opt) != 1) {
            while (getchar() != '\n');
            continue;
        }
        getchar();

        if (process_command(sock_fd, opt) < 0)
            break;
        if (opt == 4)
            break;
    }

    close(sock_fd);
    printf("[ADMIN CLIENT] Deconectat.\n");
    return 0;
}