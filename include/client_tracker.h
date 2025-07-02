// === include/client_tracker.h ===
#ifndef CLIENT_TRACKER_H
#define CLIENT_TRACKER_H

#include <pthread.h>
#include <netinet/in.h>

#define MAX_CLIENTS 100
#define IP_LENGTH INET_ADDRSTRLEN

typedef struct {
    char ip[IP_LENGTH];
    int socket_fd;
} ClientInfo;

extern ClientInfo client_list[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t client_mutex;

void add_client(const char *ip, int sock_fd);
void remove_client_by_ip(const char *ip);
void get_client_list(char *dest_buffer, size_t max_len);

#endif