#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../include/job_queue.h"

#define MAX_JOBS 100

static Job queue[MAX_JOBS];
static int front = 0, rear = 0, count = 0;

static pthread_mutex_t lock;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Variabila noua pentru a stoca job-ul curent
static Job* currently_processing_job = NULL;

extern volatile sig_atomic_t running;

void job_queue_init() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);
}

void job_queue_destroy() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
}

int job_queue_enqueue(Job job) {
    pthread_mutex_lock(&lock);
    while (count == MAX_JOBS) {
        pthread_cond_wait(&not_full, &lock);
    }
    queue[rear] = job;
    rear = (rear + 1) % MAX_JOBS;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
    return 0;
}

int job_queue_dequeue(Job* job_out) {
    pthread_mutex_lock(&lock);
    while (count == 0) {
        if (!running) {
            pthread_mutex_unlock(&lock);
            return -1;
        }
        pthread_cond_wait(&not_empty, &lock);
    }
    *job_out = queue[front];
    front = (front + 1) % MAX_JOBS;
    count--;

    currently_processing_job = (Job*)malloc(sizeof(Job));
    if (currently_processing_job) {
        memcpy(currently_processing_job, job_out, sizeof(Job));
    }

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return 0;
}

void job_queue_clear_processing_job() {
    pthread_mutex_lock(&lock);
    if (currently_processing_job) {
        free(currently_processing_job);
        currently_processing_job = NULL;
    }
    pthread_mutex_unlock(&lock);
}

// Functia get_job_list este constienta de ambele stari
void get_job_list(char *dest_buffer, size_t max_len) {
    pthread_mutex_lock(&lock);

    dest_buffer[0] = '\0'; // Incepem cu un buffer gol

    // 1. Verificam daca un job este in procesare
    if (currently_processing_job != NULL) {
        snprintf(dest_buffer, max_len, "Job in procesare:\n - [%s] pentru '%s'\n\n",
                 currently_processing_job->command, currently_processing_job->input_file);
    }

    // 2. Verificam daca sunt job-uri in coada de asteptare
    if (count == 0) {
        if (currently_processing_job == NULL) {
            snprintf(dest_buffer, max_len, "Niciun job activ\n");
        } else {
            strncat(dest_buffer, "Niciun job in coada de asteptare.\n", max_len - strlen(dest_buffer) - 1);
        }
    } else {
        char queue_header[128];
        snprintf(queue_header, sizeof(queue_header), "Job-uri in coada (%d):\n", count);
        strncat(dest_buffer, queue_header, max_len - strlen(dest_buffer) - 1);

        int current_index = front;
        for (int i = 0; i < count; i++) {
            char job_info[1024];
            snprintf(job_info, sizeof(job_info), " - [%s] pentru '%s'\n",
                     queue[current_index].command, queue[current_index].input_file);

            if (strlen(dest_buffer) + strlen(job_info) < max_len) {
                strncat(dest_buffer, job_info, max_len - strlen(dest_buffer) - 1);
            } else {
                break;
            }
            current_index = (current_index + 1) % MAX_JOBS;
        }
    }

    pthread_mutex_unlock(&lock);
}