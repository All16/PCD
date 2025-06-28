#include "../include/ffmpeg_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#define COMMAND_BUFFER_SIZE 2048  // Mărit pentru a preveni trunchieri

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

    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" -vn -acodec libmp3lame -q:a 2 \"%s\"",
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

    char output[PATH_MAX];
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

int ffmpeg_concat(const char* input1, const char* input2, const char* output) {
    char cmd[COMMAND_BUFFER_SIZE];

    // Pas 1: Convertim fiecare input într-un fișier .ts temporar
    char temp1[] = "temp1.ts";
    char temp2[] = "temp2.ts";

    snprintf(cmd, sizeof(cmd),
        "ffmpeg -y -i \"%s\" -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\" && "
        "ffmpeg -y -i \"%s\" -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\" && "
        // Pas 2: Concatenăm cele două fișiere .ts într-un mp4 final
        "ffmpeg -y -i \"concat:%s|%s\" -c copy -bsf:a aac_adtstoasc \"%s\" && "
        // Pas 3: Ștergem fișierele temporare
        "rm -f \"%s\" \"%s\"",
        input1, temp1,
        input2, temp2,
        temp1, temp2, output,
        temp1, temp2
    );

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", cmd);
    return system(cmd);
}

int ffmpeg_change_resolution(const char* input, const char* resolution, const char* output) {
    if (!input || !resolution || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru schimbare rezoluție.\n");
        return -1;
    }

    char command[COMMAND_BUFFER_SIZE];
    int ret = snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" -vf scale=%s \"%s\"",
        input, resolution, output);

    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Comanda depășește dimensiunea bufferului.\n");
        return -1;
    }

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", command);
    return system(command);
}

int ffmpeg_cut_out(const char* input, const char* start, const char* end, const char* output) {
    char cmd[1024];

    snprintf(cmd, sizeof(cmd),
        "ffmpeg -y -i \"%s\" -ss 0 -to %s -c copy part1.mp4 && "
        "ffmpeg -y -i \"%s\" -ss %s -c copy part2.mp4 && "
        "echo \"file 'part1.mp4'\" > parts.txt && "
        "echo \"file 'part2.mp4'\" >> parts.txt && "
        "ffmpeg -y -f concat -safe 0 -i parts.txt -c copy \"%s\" && "
        "rm -f part1.mp4 part2.mp4 parts.txt",
        input, start, input, end, output);

    fprintf(stderr, "[FFMPEG] Execut: %s\n", cmd);
    return system(cmd);
}

int ffmpeg_speed_segment(const char *input, const char *start, const char *end, const char *factor, const char *output) {
    char command[1024];

    snprintf(command, sizeof(command),
        "ffmpeg -y -i \"%s\" -filter_complex \""
        "[0:v]trim=start=0:end=%s,setpts=PTS-STARTPTS[v1];"
        "[0:v]trim=start=%s:end=%s,setpts=PTS-STARTPTS,fps=30,setpts=PTS/%s[v2];"
        "[0:v]trim=start=%s,setpts=PTS-STARTPTS[v3];"
        "[0:a]atrim=start=0:end=%s,asetpts=PTS-STARTPTS[a1];"
        "[0:a]atrim=start=%s:end=%s,asetpts=PTS-STARTPTS,atempo=%s[a2];"
        "[0:a]atrim=start=%s,asetpts=PTS-STARTPTS[a3];"
        "[v1][v2][v3]concat=n=3:v=1:a=0[v];"
        "[a1][a2][a3]concat=n=3:v=0:a=1[a]\" "
        "-map \"[v]\" -map \"[a]\" \"%s\"",
        input, start, start, end, factor,
        end, start, start, end, factor,
        end, output);

    return system(command);
}
