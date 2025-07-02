#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h> // Necesar pentru sig_atomic_t

#include "../include/user_interface.h"

#define INCOMING "../videos/incoming/"
#define PROCESSING "../videos/processing/"
#define OUTGOING "../videos/outgoing/"

#define SERVER_PORT 5001
#define SERVER_IP "127.0.0.1"

int sock_global = -1;
// ### START MODIFICARE: Definim un flag 'running' local pentru client ###
// Am inlocuit 'extern' cu o definitie. Acest flag controleaza DOAR clientul.
volatile sig_atomic_t running = 1;
// ### END MODIFICARE ###

void move_file(const char *src, const char *dest) {
    char command[512];
    snprintf(command, sizeof(command), "mv \"%s\" \"%s\"", src, dest);
    system(command);
}

void copy_file(const char *src, const char *dest) {
    char command[512];
    snprintf(command, sizeof(command), "cp \"%s\" \"%s\"", src, dest);
    system(command);
}

void create_directories() {
    system("mkdir -p " INCOMING);
    system("mkdir -p " PROCESSING);
    system("mkdir -p " OUTGOING);
}

void connect_to_inet_server() {
    struct sockaddr_in serv_addr;
    sock_global = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_global < 0) {
        perror("[USER] Eroare la creare socket INET");
        return;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);
    if (connect(sock_global, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[USER] Eroare la conectare");
        close(sock_global);
        sock_global = -1;
        return;
    }
}

// Functie pentru a procesa inputul de la utilizator
void handle_user_input() {
    char input_buffer[16];
    int opt;

    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        running = 0; // Oprim la EOF (Ctrl+D)
        return;
    }
    opt = atoi(input_buffer);

    if (opt == 1) {
        char filename[256];
        get_user_input(filename, sizeof(filename), "Nume fișier video: ");

        char source_path[512];
        char processing_path[512];
        snprintf(source_path, sizeof(source_path), "%s%s", INCOMING, filename);
        snprintf(processing_path, sizeof(processing_path), "%s%s", PROCESSING, filename);

        struct stat st;
        if (stat(source_path, &st) != 0) {
            printf("\n[EROARE] Fișierul sursă '%s' nu a fost găsit!\n", filename);
            return;
        }

        copy_file(source_path, processing_path);
        // ... restul logicii pentru meniul de editare ...
        printf("\n[INFO] Aici ar urma meniul de editare pentru %s...\n", filename);

    } else if (opt == 2) {
        printf("Ieșire solicitată de utilizator.\n");
        running = 0; // Folosim flag-ul local
    } else {
        printf("Opțiune invalidă.\n");
    }
}


int main() {
    create_directories();
    printf("[INFO] Conectare la server...\n");
    connect_to_inet_server();

    if (sock_global < 0) {
        printf("[EROARE] Nu s-a putut stabili conexiunea. Programul se va inchide.\n");
        return 1;
    }
    printf("[INFO] Conectare reusita! Astept comenzi...\n");

    while (running) {
        print_main_menu();

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // Monitorizam tastatura
        FD_SET(sock_global, &read_fds);  // Monitorizam socket-ul serverului

        int max_fd = (sock_global > STDIN_FILENO) ? sock_global : STDIN_FILENO;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            // Verificam daca select a fost intrerupt de un semnal (ex: Ctrl+C)
            if (running == 0) break;
            perror("select error");
            break;
        }

        if (FD_ISSET(sock_global, &read_fds)) {
            char server_buffer[1024];
            ssize_t n = read(sock_global, server_buffer, sizeof(server_buffer) - 1);

            if (n <= 0) {
                printf("\n[SYSTEM] Serverul a inchis conexiunea. Programul se va opri.\n");
                running = 0; // Oprim bucla
                break;
            }

            server_buffer[n] = '\0';
            printf("\n--- MESAJ DE LA SERVER ---\n%s\n--------------------------\n", server_buffer);

            if (strstr(server_buffer, "deconectat")) {
                running = 0; // Oprim bucla
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            handle_user_input();
        }
    }

    close(sock_global);
    printf("[INFO] Programul s-a incheiat.\n");
    return 0;
}
