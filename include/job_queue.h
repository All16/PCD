#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>
#include <stddef.h>

// Structura care defineste un job de procesare
typedef struct Job {
    char command[64];
    char input_file[512];
    char input_file2[512];
    char output_file[512];
    char args[256];
} Job;

// Functiile existente
void job_queue_init();
void job_queue_destroy();
int job_queue_enqueue(Job job);
int job_queue_dequeue(Job* job_out);

// Functia noua pentru a obtine lista de job-uri
void get_job_list(char *dest_buffer, size_t max_len);

// Functia pentru a semnala terminarea unui job
void job_queue_clear_processing_job();

#endif // JOB_QUEUE_H