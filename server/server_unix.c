// === server/server_unix.c ===
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
                const char *welcome = "[UNIX] Bine ai venit la interfata admin!\n";
                const char *debug = "[UNIX] Aștept comanda de la admin client...\n";
                write(clientfd, welcome, strlen(welcome));
                write(clientfd, debug, strlen(debug));

                char buffer[128] = {0};
                read(clientfd, buffer, sizeof(buffer));
                buffer[strcspn(buffer, "\n")] = 0;
                write(clientfd, buffer, strlen(buffer));

                if (strcmp(buffer, "LIST") == 0) {
                    char response[512];
                    get_client_list(response, sizeof(response));
                    write(clientfd, response, strlen(response));
                }
                else {
                    write(clientfd, "OK\n", 4);
                }

                close(clientfd);
            }
        }
    }

    close(sockfd);
    printf("[UNIX] Fir UNIX oprit curat.\n");
    pthread_exit(NULL);
}