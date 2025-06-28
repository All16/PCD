#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/user_interface.h"

#define INCOMING "videos/incoming/"
#define PROCESSING "videos/processing/"
#define OUTGOING "videos/outgoing/"

void move_file(const char *src, const char *dest) {
    char command[512];
    snprintf(command, sizeof(command), "mv \"%s\" \"%s\"", src, dest);
    system(command);
}

void copy_file(const char *src, const char *dest) {
    char command[512];
    snprintf(command, sizeof(command), "cp \"%s\" \"%s\"", src, dest);
    system(command);
}

int main() {
    char filename[256];
    int opt;

    while (1) {
        print_main_menu();  // afișează: 1. Edit video, 2. Exit
        scanf("%d", &opt);
        getchar();  // elimină newline

        if (opt == 1) {
            get_user_input(filename, sizeof(filename), "Nume fișier video: ");

            char source_path[512], processing_path[512];
            snprintf(source_path, sizeof(source_path), "%s%s", INCOMING, filename);
            snprintf(processing_path, sizeof(processing_path), "%s%s", PROCESSING, filename);

            // Copiază fișierul în processing
            copy_file(source_path, processing_path);
            printf("[INFO] Fișier copiat în processing: %s\n", processing_path);

            while (1) {
                print_edit_menu();  // meniul cu opțiuni: cut, extract_audio, etc.
                int subopt;
                scanf("%d", &subopt);
                getchar();

                if (subopt == 1) {  // Cut
                    char start[64], end[64];
                    get_user_input(start, sizeof(start), "Start time (ex: 00:00:10): ");
                    get_user_input(end, sizeof(end), "End time (ex: 00:00:20): ");

                    char cmd[512];
                    snprintf(cmd, sizeof(cmd),
                        "python3 client_rest/rest_client.py cut %s %s %s processing",
                        filename, start, end);
                    system(cmd);
                }
                else if (subopt == 2) {  // Extract audio
                    char cmd[512];
                    snprintf(cmd, sizeof(cmd),
                        "python3 client_rest/rest_client.py extract_audio %s processing",
                        filename);
                    system(cmd);
                }
                else if (subopt == 3) {  // Concat
                    char file2[256];
                    get_user_input(file2, sizeof(file2), "Al doilea fișier (din incoming): ");
                    char cmd[1024];
                    snprintf(cmd, sizeof(cmd),
                            "python3 client_rest/rest_client.py concat %s %s processing",
                            filename, file2);
                    system(cmd);

                }
                else if (subopt == 4) {  // Change resolution
                    char width[32], height[32];
                    get_user_input(width, sizeof(width), "Lățime: ");
                    get_user_input(height, sizeof(height), "Înălțime: ");

                    char resolution[64];
                    snprintf(resolution, sizeof(resolution), "%s:%s", width, height);

                    char cmd[512];
                    snprintf(cmd, sizeof(cmd),
                        "python3 client_rest/rest_client.py change_resolution %s %s processing",
                        filename, resolution);
                    system(cmd);
                }
                else if (subopt == 0) {
                    // La final, mută fișierul principal în outgoing
                    char proc_path[512], final_path[512];
                    snprintf(proc_path, sizeof(proc_path), "%s%s", PROCESSING, filename);
                    snprintf(final_path, sizeof(final_path), "%s%s", OUTGOING, filename);
                    move_file(proc_path, final_path);
                    printf("[INFO] Fișier video mutat în outgoing: %s\n", final_path);

                    // Dacă există și mp3 asociat, mută și acela
                    char base[256], *dot = strrchr(filename, '.');
                    if (dot) {
                        strncpy(base, filename, dot - filename);
                        base[dot - filename] = '\0';
                    } else {
                        strncpy(base, filename, sizeof(base));
                    }

                    char mp3_proc[512], mp3_out[512];
                    snprintf(mp3_proc, sizeof(mp3_proc), "%s%s.mp3", PROCESSING, base);
                    snprintf(mp3_out, sizeof(mp3_out), "%s%s.mp3", OUTGOING, base);

                    struct stat st;
                    if (stat(mp3_proc, &st) == 0) {
                        move_file(mp3_proc, mp3_out);
                        printf("[INFO] Fișier audio mutat în outgoing: %s\n", mp3_out);
                    }

                    break;
                }
                else {
                    printf("Opțiune invalidă.\n");
                }
            }
        }
        else if (opt == 2) {
            printf("Ieșire.\n");
            break;
        }
        else {
            printf("Opțiune invalidă.\n");
        }
    }

    return 0;
}
