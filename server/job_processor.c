#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <linux/limits.h>
#include <stdlib.h>

#include "../include/job_queue.h"
#include "../include/ffmpeg_wrapper.h"
#include "../include/common.h"

extern volatile sig_atomic_t running;

void process_job(const char *job_type, const char *input, const char *output, const char *extra) {
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];

    // Convertim căile relative în căi absolute
    if (strstr(input, "/") == NULL) {
        snprintf(input_path, sizeof(input_path), "/home/vboxuser/PCD/Proiect/PCD/%s", input);
    } else {
        strncpy(input_path, input, sizeof(input_path));
    }

    if (strstr(output, "/") == NULL) {
        snprintf(output_path, sizeof(output_path), "/home/vboxuser/PCD/Proiect/PCD/%s", output);
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
    char path1[PATH_MAX], path2[PATH_MAX];
    realpath(input, path1); // input e deja file1
    snprintf(path2, sizeof(path2), "/home/vboxuser/PCD/Proiect/PCD/videos/incoming/%s", file2);

    ffmpeg_concat(path1, path2, output_path);
}
 else {
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
        else if (strcmp(job_type, "cut_except") == 0) {
    char start[64], end[64];
    if (sscanf(extra, "%63s %63s", start, end) != 2) {
        fprintf(stderr, "[PROC] Format invalid pentru 'cut_except'. Așteptat: start end\n");
        return;
    }

    // Suprascrie fișierul original în processing
    char actual_output[PATH_MAX];
    snprintf(actual_output, sizeof(actual_output), "%s", input_path);

    log_message("PROC", "Execut cut_except %s [fără %s–%s]", input_path, start, end);
    ffmpeg_cut_out(input_path, start, end, actual_output);
}
    else if (strcmp(job_type, "speed_segment") == 0) {
    char start[64], end[64], factor[32];
    if (sscanf(extra, "%63s %63s %31s", start, end, factor) != 3) {
        fprintf(stderr, "[PROC] Format invalid pentru speed_segment. Așteptat: start end factor\n");
        return;
    }

    log_message("PROC", "Execut speed_segment %s între %s și %s cu factor %s", input_path, start, end, factor);
    ffmpeg_speed_segment(input_path, start, end, factor, output_path);
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
