#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include "../include/client_tracker.h"

#define UNIX_SOCKET_PATH "/tmp/vedit_admin_socket"

extern volatile sig_atomic_t running;

void* handle_admin_socket(void* arg) {
    int sockfd, clientfd;
    struct sockaddr_un addr;

    unlink(UNIX_SOCKET_PATH);
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 1);

    printf("[UNIX] Server UNIX admin asculta pe %s\n", UNIX_SOCKET_PATH);

    fd_set fds;
    struct timeval timeout;

    while (running) {
        pthread_testcancel();

        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(sockfd + 1, &fds, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(sockfd, &fds)) {
            clientfd = accept(sockfd, NULL, NULL);
            if (clientfd >= 0) {
                printf("[UNIX] Client admin acceptat.\n");
                const char *welcome = "[UNIX] Bine ai venit la interfata admin!\n";
                write(clientfd, welcome, strlen(welcome));

                char buffer[256];
                ssize_t n_read;
                while ((n_read = read(clientfd, buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[n_read] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = 0;

                    printf("[UNIX] Am primit comanda: '%s'\n", buffer);

                    if (strcmp(buffer, "exit") == 0) {
                        break;
                    }

                    if (strcmp(buffer, "LIST") == 0) {
                        char response[1024] = {0};
                        get_client_list(response, sizeof(response));
                        strncat(response, "\n", sizeof(response) - strlen(response) - 1);
                        write(clientfd, response, strlen(response));
                    }
                        // ### START MODIFICARE: Logica pentru KICK ###
                    else if (strncmp(buffer, "KICK ", 5) == 0) {
                        // Extragem IP-ul de dupa "KICK "
                        char* ip_to_kick = buffer + 5;
                        printf("[UNIX] Cerere de deconectare pentru IP: %s\n", ip_to_kick);

                        // Apelam functia care inchide socket-ul si sterge clientul
                        remove_client_by_ip(ip_to_kick);

                        // Trimitem mesajul de confirmare
                        char response[128];
                        snprintf(response, sizeof(response), "Clientul cu IP-ul %s a fost deconectat.\n", ip_to_kick);
                        write(clientfd, response, strlen(response));
                    }
                        // ### END MODIFICARE ###
                    else {
                        const char* ok_response = "OK\n";
                        write(clientfd, ok_response, strlen(ok_response));
                    }
                    memset(buffer, 0, sizeof(buffer));
                }

                printf("[UNIX] Inchidere conexiune admin.\n");
                close(clientfd);
            }
        }
    }

    close(sockfd);
    printf("[UNIX] Fir UNIX oprit curat.\n");
    pthread_exit(NULL);
}