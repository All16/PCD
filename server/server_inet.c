// === server/server_inet.c ===
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include "../include/client_tracker.h"

#define PORT 5001

extern volatile sig_atomic_t running;

void* handle_user_clients(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("[INET] Server INET user ascultă pe port %d\n", PORT);

    fd_set fds;
    struct timeval timeout;

    while (running) {
        pthread_testcancel();

        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(server_fd + 1, &fds, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(server_fd, &fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd >= 0) {
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                printf("[INET] Conectat client: %s\n", ip_str);

                add_client(ip_str, client_fd);

                printf("[INET] Clienti conectati: %d\n", client_count);
                char buffer[256] = {0};
                int n = read(client_fd, buffer, sizeof(buffer));
                if (n > 0) {
                    printf("[INET] Comandă: %s\n", buffer);
                    write(client_fd, "[Server] OK\n", 13);
                }

                if (n <= 0) {
                    remove_client_by_ip(ip_str);
                    close(client_fd);
                }
            }
        }
    }

    close(server_fd);
    printf("[INET] Fir INET oprit curat.\n");
    pthread_exit(NULL);
}

/*int main() {
    handle_user_clients(NULL);
    return 0;
}*/
