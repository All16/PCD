#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define PORT 5001

extern volatile sig_atomic_t running;

void* handle_user_clients(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

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
        pthread_testcancel();  // 👈 permite terminare curată

        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(server_fd + 1, &fds, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(server_fd, &fds)) {
            client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
            if (client_fd >= 0) {
                char buffer[256] = {0};
                int n = read(client_fd, buffer, sizeof(buffer));
                if (n > 0) {
                    printf("[INET] Comandă de la client: %s\n", buffer);
                    write(client_fd, "[Server] Comandă primită\n", 26);
                }
                close(client_fd);
            }
        }
    }

    close(server_fd);
    printf("[INET] Fir INET oprit curat.\n");
    pthread_exit(NULL);
}

