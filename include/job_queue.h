#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>

#define MAX_JOBS 100

// Structura unui job (poți adăuga mai multe câmpuri ulterior)
typedef struct {
    char input_file[256];
    char command[64];     // ex: "cut", "convert"
    char args[256];       // ex: "00:00:10 00:00:20"
    char output_file[256];
    int client_socket;    // socketul clientului care a trimis comanda
} Job;

// Prototipuri funcții
void job_queue_init();
void job_queue_destroy();
int job_queue_enqueue(Job job);
int job_queue_dequeue(Job* job_out);
extern pthread_cond_t not_empty;

#endif