#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../include/ffmpeg_wrapper.h"

int ffmpeg_cut(const char* input, const char* start, const char* end, const char* output) {
    if (!input || !start || !end || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru cut.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    char temp_output[512];
    snprintf(temp_output, sizeof(temp_output), "tmp/cut_tmp.mp4");

    char command[1024];
    snprintf(command, sizeof(command),
             "ffmpeg -y -i \"%s\" -ss %s -to %s -c:v libx264 -c:a aac \"%s\"",
             input, start, end, temp_output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);

    int ret = system(command);
    if (ret == 0) {
        rename(temp_output, output);
    }

    return ret;
}

int ffmpeg_extract_audio(const char* input, const char* output) {
    if (!input || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru extract_audio.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    char temp_output[512];
    snprintf(temp_output, sizeof(temp_output), "tmp/audio_tmp.mp3");

    char command[1024];
    snprintf(command, sizeof(command),
             "ffmpeg -y -i \"%s\" -q:a 0 -map a \"%s\"",
             input, temp_output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);

    int ret = system(command);
    if (ret == 0) {
        rename(temp_output, output);
    }

    return ret;
}

int ffmpeg_change_resolution(const char* input, const char* resolution, const char* output) {
    if (!input || !resolution || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru change_resolution.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    char temp_output[512];
    snprintf(temp_output, sizeof(temp_output), "tmp/res_tmp.mp4");

    char command[1024];
    snprintf(command, sizeof(command),
             "ffmpeg -y -i \"%s\" -vf scale=%s -c:a copy \"%s\"",
             input, resolution, temp_output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);

    int ret = system(command);
    if (ret == 0) {
        rename(temp_output, output);
    }

    return ret;
}

int ffmpeg_concat(const char* file1, const char* file2, const char* output) {
    if (!file1 || !file2 || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru concat.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    // Normalizează file2 (timestamps)
    char normalized_file2[PATH_MAX];
    snprintf(normalized_file2, sizeof(normalized_file2), "tmp/normalized.mp4");

    char norm_cmd[2048];
    snprintf(norm_cmd, sizeof(norm_cmd),
        "ffmpeg -y -i \"%s\" -fflags +genpts -reset_timestamps 1 -c copy \"%s\"",
        file2, normalized_file2);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", norm_cmd);
    if (system(norm_cmd) != 0) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Eroare normalizare file2\n");
        return -1;
    }

    // Creare concat_list.txt
    FILE* list = fopen("tmp/concat_list.txt", "w");
    if (!list) {
        perror("[FFMPEG_WRAPPER] Eroare creare concat_list.txt");
        return -1;
    }

    char abs_file1[PATH_MAX];
    char abs_file2[PATH_MAX];
    realpath(file1, abs_file1);
    realpath(normalized_file2, abs_file2);

    fprintf(list, "file '%s'\n", abs_file1);
    fprintf(list, "file '%s'\n", abs_file2);
    fclose(list);

    // Comandă de concat
    char temp_output[PATH_MAX];
    snprintf(temp_output, sizeof(temp_output), "tmp/concat_output.mp4");

    char command[2048];
    snprintf(command, sizeof(command),
        "ffmpeg -y -f concat -safe 0 -i tmp/concat_list.txt -c:v libx264 -c:a aac -strict experimental \"%s\"",
        temp_output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);
    int ret = system(command);

    if (ret == 0) {
        rename(temp_output, output);
    }

    // Curățare fișiere temporare
    remove("tmp/concat_list.txt");
    remove(normalized_file2);
    remove(temp_output);  // în caz că nu s-a mutat

    return ret;
}



int ffmpeg_convert(const char* input, const char* format, const char* output) {
    if (!input || !format || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru convert.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    char temp_output[512];
    snprintf(temp_output, sizeof(temp_output), "tmp/convert_tmp.%s", format);

    char command[1024];
    snprintf(command, sizeof(command),
             "ffmpeg -y -i \"%s\" \"%s\"",
             input, temp_output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);

    int ret = system(command);
    if (ret == 0) {
        rename(temp_output, output);
    }

    return ret;
}
