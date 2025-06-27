#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h> // For sig_atomic_t
#include "../include/job_queue.h"

#define MAX_JOBS 100

static Job queue[MAX_JOBS];
static int front = 0;
static int rear = 0;
static int count = 0;

static pthread_mutex_t lock;
pthread_cond_t not_empty;
pthread_cond_t not_full;

extern volatile sig_atomic_t running; // importă variabila globală

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
            return -1; // semnal de oprire
        }
        pthread_cond_wait(&not_empty, &lock);
    }

    *job_out = queue[front];
    front = (front + 1) % MAX_JOBS;
    count--;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return 0;
}
