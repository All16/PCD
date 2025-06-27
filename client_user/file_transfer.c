#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/file_transfer.h"

int send_file(int sock_fd, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("[CLIENT] Eroare la deschiderea fișierului");
        return -1;
    }

    char buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sock_fd, buffer, bytes_read) <= 0) {
            perror("[CLIENT] Eroare la trimiterea fișierului");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

int receive_file(int sock_fd, const char *file_path) {
    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("[CLIENT] Eroare la crearea fișierului");
        return -1;
    }

    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(sock_fd, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytes_read, file) != (size_t)bytes_read) {
            perror("[CLIENT] Eroare la scrierea în fișier");
            fclose(file);
            return -1;
        }
    }

    if (bytes_read < 0) {
        perror("[CLIENT] Eroare la citirea fișierului");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}