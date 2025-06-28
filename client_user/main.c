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
    char input[256], input2[256], start[64], end[64];
    char buffer[1024];

    while (1) {
        print_menu();
        scanf("%d", &opt);
        getchar(); // Consumăm newline

        if (opt == 1) {
            get_user_input(input, sizeof(input), "Nume fișier (ex: test.mp4): ");
            get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
            get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");
            snprintf(buffer, sizeof(buffer), 
                "python3 /home/vboxuser/PCD/Proiect/PCD/client_rest/rest_client.py cut %s %s %s",
                input, start, end);
            system(buffer);
        } 
        else if (opt == 2) {
            get_user_input(input, sizeof(input), "Nume fișier (ex: test.mp4): ");
            snprintf(buffer, sizeof(buffer), 
                "python3 /home/vboxuser/PCD/Proiect/PCD/client_rest/rest_client.py extract_audio %s",
                input);
            system(buffer);
        }
        else if (opt == 3) {
            get_user_input(input, sizeof(input), "Primul fișier video (ex: test.mp4): ");
            get_user_input(input2, sizeof(input2), "Al doilea fișier video (ex: test1.mp4): ");
            snprintf(buffer, sizeof(buffer),
                "python3 /home/vboxuser/PCD/Proiect/PCD/client_rest/rest_client.py concat %s %s",
                input, input2);
            system(buffer);
        } 
        else if (opt == 4) {
            get_user_input(input, sizeof(input), "Nume fișier (ex: test.mp4): ");
            char width[16], height[16];
            get_user_input(width, sizeof(width), "Lățime (ex: 640): ");
            get_user_input(height, sizeof(height), "Înălțime (ex: 360): ");
            snprintf(buffer, sizeof(buffer), 
                "python3 /home/vboxuser/PCD/Proiect/PCD/client_rest/rest_client.py change_resolution %s %s %s",
                input, width, height);
            system(buffer);
}

        else if (opt == 0) {
            printf("[CLIENT] Deconectare...\n");
            break;
        } 
        else {
            printf("Opțiune invalidă.\n");
        }
    }

    close(sock_fd);
    return 0;
}