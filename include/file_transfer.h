#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

int send_file(int sock_fd, const char *file_path);
int receive_file(int sock_fd, const char *file_path);

#endif