// === client_admin/main.c ===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>  // Necesar pentru fcntl
#include <errno.h>  // Necesar pentru errno

#include "../include/admin_interface.h"
#include "../include/admin_commands.h"

#define ADMIN_SOCKET_PATH "/tmp/vedit_admin_socket"

/**
 * @brief Citeste tot raspunsul disponibil de la server.
 *
 * Seteaza temporar socket-ul pe non-blocking pentru a citi tot ce se afla
 * in buffer-ul de retea, pana cand acesta este gol. Apoi il seteaza inapoi
 * pe blocking. Aceasta metoda previne desincronizarea.
 * @param sock_fd Socket-ul de pe care se citeste.
 */
void read_full_response(int sock_fd) {
    char buffer[4096];
    ssize_t total_bytes_read = 0;
    ssize_t n;

    // Golim buffer-ul inainte de utilizare
    memset(buffer, 0, sizeof(buffer));

    // Setam socket-ul pe non-blocking
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    // Citim intr-o bucla pana cand buffer-ul este gol
    while (1) {
        n = read(sock_fd, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read - 1);
        if (n > 0) {
            total_bytes_read += n;
        } else if (n == 0) {
            // Conexiunea a fost inchisa de server
            printf("[SYSTEM] Serverul a inchis conexiunea.\n");
            break;
        } else { // n < 0
            // Verificam daca eroarea este pentru ca nu mai sunt date de citit
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Nu mai sunt date de citit pentru moment, am terminat
                break;
            } else {
                // O eroare reala de citire
                perror("[ADMIN] Eroare la citirea raspunsului");
                break;
            }
        }
    }

    // Setam socket-ul inapoi pe blocking
    fcntl(sock_fd, F_SETFL, flags);

    // Afisam ce am citit, doar daca am citit ceva
    if (total_bytes_read > 0) {
        printf("[SERVER]:\n%s", buffer);
    }
}

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

    // Asteptam putin si citim mesajul de bun venit
    usleep(100000); // 100ms
    read_full_response(sock_fd);

    while (!handleLogin());

    int opt;
    while (1) {
        printMenu();
        if (scanf("%d", &opt) != 1) {
            while (getchar() != '\n'); // Curatam buffer-ul de input
            continue;
        }
        getchar(); // Consumam newline-ul lasat de scanf

        if (process_command(sock_fd, opt) < 0) {
            break;
        }
        if (opt == 4) { // Optiunea de iesire
            break;
        }

        // Asteptam putin ca serverul sa proceseze si sa trimita raspunsul
        usleep(100000); // 100ms
        read_full_response(sock_fd);
    }

    close(sock_fd);
    printf("[ADMIN CLIENT] Deconectat.\n");
    return 0;
}