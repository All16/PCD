#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <linux/limits.h>

#include "../include/job_queue.h"
#include "../include/ffmpeg_wrapper.h"
#include "../include/common.h"

extern volatile sig_atomic_t running;

void process_job(const char *job_type, const char *input, const char *output, const char *extra) {
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];

    if (strstr(input, "/") == NULL) {
        snprintf(input_path, sizeof(input_path), "/home/User/Desktop/Proiect/videos/incoming/%s", input);
    } else {
        strncpy(input_path, input, sizeof(input_path));
    }

    if (strstr(output, "/") == NULL) {
        snprintf(output_path, sizeof(output_path), "/home/User/Desktop/Proiect/videos/outgoing/%s", output);
    } else {
        strncpy(output_path, output, sizeof(output_path));
    }

    if (strcmp(job_type, "cut") == 0) {
        char start[64], end[64];
        if (sscanf(extra, "%63s %63s", start, end) != 2) {
            fprintf(stderr, "[PROC] Format invalid pentru 'cut'. Așteptat: start end\n");
            return;
        }
        log_message("PROC", "Execut cut %s -> %s [%s to %s]", input_path, output_path, start, end);
        ffmpeg_cut(input_path, start, end, output_path);
    }
    else if (strcmp(job_type, "extract_audio") == 0) {
        log_message("PROC", "Execut extract_audio %s -> %s", input_path, output_path);
        ffmpeg_extract_audio(input_path, output_path);
    }
    else if (strcmp(job_type, "convert") == 0) {
        log_message("PROC", "Execut convert %s -> %s (format %s)", input_path, output_path, extra);
        ffmpeg_convert(input_path, extra, output_path);
    }
    else if (strcmp(job_type, "concat") == 0) {
        char file1[128], file2[128];
        if (sscanf(extra, "%127s %127s", file1, file2) == 2) {
            char path1[256], path2[256];
            snprintf(path1, sizeof(path1), "/home/User/Desktop/Proiect/videos/incoming/%s", file1);
            snprintf(path2, sizeof(path2), "/home/User/Desktop/Proiect/videos/incoming/%s", file2);
            log_message("PROC", "Execut concat %s + %s -> %s", path1, path2, output_path);
            ffmpeg_concat(path1, path2, output_path);
        } else {
            fprintf(stderr, "[PROC] Format invalid pentru concat.\n");
        }
    }
    else if (strcmp(job_type, "change_resolution") == 0) {
        log_message("PROC", "Execut change_resolution %s -> %s (rezoluție %s)", input_path, output_path, extra);
        int ret = ffmpeg_change_resolution(input_path, extra, output_path);
        if (ret != 0) {
            fprintf(stderr, "[PROC] Eroare la schimbarea rezoluției pentru %s\n", input_path);
        }
    }
    else {
        log_message("ERROR", "Tip job necunoscut: %s", job_type);
    }
}

void* job_processor(void* arg) {
    printf("[PROC] Firul de procesare pornit...\n");

    while (running) {
        pthread_testcancel();

        Job job;
        if (job_queue_dequeue(&job) == -1)
            break;

        log_message("JOB", "Procesare job: %s %s -> %s", job.command, job.input_file, job.output_file);
        process_job(job.command, job.input_file, job.output_file, job.args);
    }

    printf("[PROC] Firul de procesare s-a oprit complet.\n");
    pthread_exit(NULL);
}
