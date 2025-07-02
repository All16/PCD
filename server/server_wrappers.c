// server/server_wrappers.c
#include <pthread.h>
extern void* handle_admin_socket(void*);
extern void* handle_user_clients(void*);
extern void* job_processor(void*);
extern void* start_rest_server(void*);
