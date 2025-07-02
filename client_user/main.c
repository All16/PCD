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

void handle_edit_session() {
    clearScreen();
    char filename[256];
    get_user_input(filename, sizeof(filename), "Nume fișier video (din 'incoming'): ");

    char source_path[512], processing_path[512];
    snprintf(source_path, sizeof(source_path), "%s%s", INCOMING, filename);
    snprintf(processing_path, sizeof(processing_path), "%s%s", PROCESSING, filename);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        printf("\n[EROARE] Fișierul sursă '%s' nu a fost găsit!\n", filename);
        printf("Apasa Enter pentru a reveni la meniul principal...");
        getchar();
        return;
    }

    copy_file(source_path, processing_path);
    printf("[INFO] Fișier '%s' pregatit pentru editare.\n", filename);
    usleep(1500000);

    while (running) {
        clearScreen();
        print_edit_menu();

        char input_buffer[16];
        int subopt;
        get_user_input(input_buffer, sizeof(input_buffer), "");
        subopt = atoi(input_buffer);

        if (subopt == 0) {
            char final_path[512];
            snprintf(final_path, sizeof(final_path), "%s%s", OUTGOING, filename);
            move_file(processing_path, final_path);
            printf("[INFO] Fișier finalizat și mutat în 'outgoing': %s\n", final_path);
            printf("Apasa Enter pentru a reveni la meniul principal...");
            getchar();
            break;
        }

        // ### START MODIFICARE: Implementat logica pentru comenzi ###
        char cmd[2048]; // Buffer marit pentru comenzi complexe
        int command_sent = 1; // Flag pentru a sti daca sa asteptam input

        if (subopt == 1) {  // Cut
            char start[64], end[64];
            get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
            get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py cut \"%s\" %s %s processing", filename, start, end);
            system(cmd);
        }
        else if (subopt == 2) {  // Extract audio
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py extract_audio \"%s\" processing", filename);
            system(cmd);
        }
        else if (subopt == 3) {  // Concat
            char file2[256];
            get_user_input(file2, sizeof(file2), "Al doilea fișier (din 'incoming'): ");
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py concat \"%s\" \"%s\" processing", filename, file2);
            system(cmd);
        }
        else if (subopt == 4) {  // Change resolution
            char width[32], height[32];
            get_user_input(width, sizeof(width), "Lățime: ");
            get_user_input(height, sizeof(height), "Înălțime: ");
            char resolution[64];
            snprintf(resolution, sizeof(resolution), "%sx%s", width, height);
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py change_resolution \"%s\" %s processing", filename, resolution);
            system(cmd);
        }
        else if (subopt == 5) {  // Cut Except
            char start[64], end[64];
            get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
            get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py cut_except \"%s\" %s %s processing", filename, start, end);
            system(cmd);
        }
        else if (subopt == 6) {  // Speed Segment
            char start[64], end[64], factor[16];
            get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
            get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");
            get_user_input(factor, sizeof(factor), "Factor (ex: 2.0): ");
            snprintf(cmd, sizeof(cmd), "python3 ../client_rest/rest_client.py speed_segment \"%s\" %s %s %s processing", filename, start, end, factor);
            system(cmd);
        }
        else {
            printf("Opțiune de editare invalidă.\n");
            command_sent = 0;
        }

        if (command_sent) {
            printf("\n[INFO] Comanda a fost trimisă. Apasă Enter pentru a continua...");
            getchar();
        }
        // ### END MODIFICARE ###
    }
}

void handle_user_input() {
    char input_buffer[16];
    int opt;

    get_user_input(input_buffer, sizeof(input_buffer), "");
    opt = atoi(input_buffer);

    if (opt == 1) {
        handle_edit_session();
    } else if (opt == 2) {
        printf("Ieșire solicitată de utilizator.\n");
        running = 0;
    } else {
        printf("Opțiune invalidă. Apasa Enter pentru a reîncerca.");
        getchar();
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
    usleep(1500000);

    while (running) {
        clearScreen();
        print_main_menu();

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock_global, &read_fds);

        int max_fd = (sock_global > STDIN_FILENO) ? sock_global : STDIN_FILENO;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            if (running == 0) break;
            perror("select error");
            break;
        }

        if (FD_ISSET(sock_global, &read_fds)) {
            char server_buffer[1024];
            ssize_t n = read(sock_global, server_buffer, sizeof(server_buffer) - 1);
            if (n <= 0) {
                printf("\n[SYSTEM] Serverul a inchis conexiunea. Programul se va opri.\n");
                running = 0;
                break;
            }
            server_buffer[n] = '\0';
            clearScreen();
            printf("\n--- MESAJ DE LA SERVER ---\n%s\n--------------------------\n", server_buffer);
            printf("Apasa Enter pentru a continua...");
            getchar();
            if (strstr(server_buffer, "deconectat")) {
                running = 0;
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
