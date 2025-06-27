#include "../include/ffmpeg_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_BUFFER_SIZE 512

int ffmpeg_cut(const char* input, const char* start, const char* end, const char* output) {
    if (!input || !start || !end || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru ffmpeg_cut.\n");
        return -1;
    }

    char command[COMMAND_BUFFER_SIZE];
    int ret = snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" -ss %s -to %s -c:v libx264 -c:a aac \"%s\"",
        input, start, end, output);

    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Comanda depășește dimensiunea bufferului.\n");
        return -1;
    }

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);
    int result = system(command);
    fprintf(stderr, "[FFMPEG_WRAPPER] Exit code: %d\n", result);

    return result;
}

int ffmpeg_extract_audio(const char* input, const char* output) {
    if (!input || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru ffmpeg_extract_audio.\n");
        return -1;
    }

    char command[512];
    snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" -vn -acodec mp3 \"%s\"",
        input, output);

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);
    int result = system(command);
    fprintf(stderr, "[FFMPEG_WRAPPER] Exit code: %d\n", result);

    return result;
}

int ffmpeg_convert(const char* input, const char* format, const char* output_base) {
    if (!input || !format || !output_base) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru ffmpeg_convert.\n");
        return -1;
    }

    char output[COMMAND_BUFFER_SIZE];
    snprintf(output, sizeof(output), "%s.%s", output_base, format);

    char command[COMMAND_BUFFER_SIZE];
    int ret = snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" \"%s\"",
        input, output);

    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Comanda depășește dimensiunea bufferului.\n");
        return -1;
    }

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);
    int result = system(command);
    fprintf(stderr, "[FFMPEG_WRAPPER] Exit code: %d\n", result);

    return result;
}