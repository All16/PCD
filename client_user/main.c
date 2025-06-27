#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/user_interface.h"
#include "../include/command_sender.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;

    // Creăm socketul
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[CLIENT] Eroare creare socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Conectare la server
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[CLIENT] Eroare conectare");
        close(sock_fd);
        return 1;
    }

    printf("[CLIENT] Conectat la serverul V-Edit.\n");

    int opt;
    char input[256], start[64], end[64];
    char buffer[512];

    while (1) {
        print_menu();
        scanf("%d", &opt);
        getchar(); // Consumăm newline

        if (opt == 1) {
            get_user_input(input, sizeof(input), "Nume fișier (ex: test.mp4): ");
            get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
            get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py cut %s %s %s", input, start, end);
            system(cmd);
        } if (opt == 2) {
            get_user_input(input, sizeof(input), "Nume fișier (ex: test.mp4): ");
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py extract_audio %s", input);
            system(cmd);
        } else if (opt == 3) {
            printf("[CLIENT] Deconectare...\n");
            break;
        } else {
            printf("Opțiune invalidă.\n");
        }
    }

    close(sock_fd);
    return 0;
}