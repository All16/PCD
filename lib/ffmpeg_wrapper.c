#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../include/ffmpeg_wrapper.h"
#include "../include/common.h"

int ffmpeg_cut(const char* input, const char* start, const char* end, const char* output) {
    if (!input || !start || !output) {
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
    if (end) {
        snprintf(command, sizeof(command),
            "ffmpeg -y -i \"%s\" -ss %s -to %s -c:v libx264 -c:a aac \"%s\"",
            input, start, end, temp_output);
    } else {
        snprintf(command, sizeof(command),
            "ffmpeg -y -i \"%s\" -ss %s -c:v libx264 -c:a aac \"%s\"",
            input, start, temp_output);
    }

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

    char ts1[PATH_MAX] = "tmp/input1.ts";
    char ts2[PATH_MAX] = "tmp/input2.ts";
    char temp_output[PATH_MAX] = "tmp/concat_output.mp4";

    // 1. Convertim fișierele în .ts (remuxate complet cu stream continuu)
    char cmd1[8192];
    char cmd2[8192];
    char cmd_concat[8192];

    snprintf(cmd1, sizeof(cmd1),
        "ffmpeg -y -i \"%s\" -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\"",
        file1, ts1);

    snprintf(cmd2, sizeof(cmd2),
        "ffmpeg -y -i \"%s\" -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\"",
        file2, ts2);

    int written = snprintf(cmd_concat, sizeof(cmd_concat),
    "ffmpeg -y -i \"concat:%s|%s\" -c copy -bsf:a aac_adtstoasc \"%s\"",
    ts1, ts2, temp_output);

    if (written < 0 || written >= sizeof(cmd_concat)) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Comanda ffmpeg_concat prea lungă! Poate apărea trunchiere.\n");
        return -1;
    }


    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", cmd1);
    if (system(cmd1) != 0) return -1;

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", cmd2);
    if (system(cmd2) != 0) return -1;

    fprintf(stderr, "[FFMPEG_WRAPPER] Execut: %s\n", cmd_concat);
    int ret = system(cmd_concat);

    if (ret == 0) {
        rename(temp_output, output);
        unlink(ts1);
        unlink(ts2);
    }

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

int ffmpeg_cut_out(const char *input_file, const char *start, const char *end, const char *output_file) {
    char cut1[PATH_MAX], cut2[PATH_MAX];

    snprintf(cut1, sizeof(cut1), "tmp/cutout_part1.mp4");
    snprintf(cut2, sizeof(cut2), "tmp/cutout_part2.mp4");

    // 1. De la început până la start
    if (ffmpeg_cut(input_file, "00:00:00", start, cut1) != 0) {
        log_message("FFMPEG_WRAPPER", "Eroare la tăierea părții 0-%s", start);
        return -1;
    }

    // 2. De la end până la final
    if (ffmpeg_cut(input_file, end, NULL, cut2) != 0) {
        log_message("FFMPEG_WRAPPER", "Eroare la tăierea părții %s-final", end);
        return -1;
    }

    // 3. Concatenare
    if (ffmpeg_concat(cut1, cut2, output_file) != 0) {
        log_message("FFMPEG_WRAPPER", "Eroare la concatenarea celor două părți");
        return -1;
    }

    // Cleanup opțional
    unlink(cut1);
    unlink(cut2);

    return 0;
}

void build_atempo_filter(const char *factor_str, char *out, size_t out_size) {
    float factor = atof(factor_str);
    out[0] = '\0';

    if (factor <= 0.0f) {
        snprintf(out, out_size, "atempo=1.0");
        return;
    }

    float remaining = factor;
    while (remaining > 2.0f + 0.01f) {
        strncat(out, "atempo=2.0,", out_size - strlen(out) - 1);
        remaining /= 2.0f;
    }

    while (remaining < 0.5f - 0.01f) {
        strncat(out, "atempo=0.5,", out_size - strlen(out) - 1);
        remaining *= 2.0f;
    }

    char last[64];
    snprintf(last, sizeof(last), "atempo=%.3f", remaining);
    strncat(out, last, out_size - strlen(out) - 1);
}

int ffmpeg_speed_segment(const char *input, const char *start, const char *end, const char *factor, const char *output) {
    if (!input || !start || !end || !factor || !output) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Parametru invalid pentru speed_segment.\n");
        return -1;
    }

    if (mkdir("tmp", 0755) == -1 && errno != EEXIST) {
        perror("[FFMPEG_WRAPPER] Eroare creare folder tmp");
        return -1;
    }

    // Drumuri relative (NU folosim realpath!)
    const char *part1 = "tmp/part1.mp4";
    const char *part2 = "tmp/part2.mp4";
    const char *part2_mod = "tmp/part2_mod.mp4";
    const char *part3 = "tmp/part3.mp4";

    // 1. Taie segmentele cu -ss DUPĂ -i pentru precizie
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1),
        "ffmpeg -y -i \"%s\" -ss 00:00:00 -to %s -c:v libx264 -c:a aac \"%s\"",
        input, start, part1);
    if (system(cmd1) != 0) return -1;

    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2),
        "ffmpeg -y -i \"%s\" -ss %s -to %s -c:v libx264 -c:a aac \"%s\"",
        input, start, end, part2);
    if (system(cmd2) != 0) return -1;

    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
        "ffmpeg -y -i \"%s\" -ss %s -c:v libx264 -c:a aac \"%s\"",
        input, end, part3);
    if (system(cmd3) != 0) return -1;

    // 2. Aplică speed modificare pe part2
    char atempo[256];
    build_atempo_filter(factor, atempo, sizeof(atempo));

    char cmd_mod[2048];
    snprintf(cmd_mod, sizeof(cmd_mod),
        "ffmpeg -y -i \"%s\" -filter_complex "
        "\"[0:v]setpts=PTS/%s[v];[0:a]%s[a]\" "
        "-map \"[v]\" -map \"[a]\" -preset ultrafast \"%s\"",
        part2, factor, atempo, part2_mod);
    if (system(cmd_mod) != 0) return -1;

// === 3. Convertim fiecare parte în .ts ===
    const char *ts1 = "tmp/input1.ts";
    const char *ts2 = "tmp/input2.ts";
    const char *ts3 = "tmp/input3.ts";

    char cmd_ts1[1024], cmd_ts2[1024], cmd_ts3[1024], cmd_concat[2048];
    snprintf(cmd_ts1, sizeof(cmd_ts1),
             "ffmpeg -y -i tmp/part1.mp4 -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\"",
             ts1);
    snprintf(cmd_ts2, sizeof(cmd_ts2),
             "ffmpeg -y -i tmp/part2_mod.mp4 -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\"",
             ts2);
    snprintf(cmd_ts3, sizeof(cmd_ts3),
             "ffmpeg -y -i tmp/part3.mp4 -c:v libx264 -preset ultrafast -crf 23 -c:a aac -f mpegts \"%s\"",
             ts3);

    if (system(cmd_ts1) != 0 || system(cmd_ts2) != 0 || system(cmd_ts3) != 0) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Eroare la conversia în .ts\n");
        return -1;
    }

    // === 4. Concatenează cele 3 .ts într-un .mp4
    snprintf(cmd_concat, sizeof(cmd_concat),
             "ffmpeg -y -i \"concat:%s|%s|%s\" -c copy -bsf:a aac_adtstoasc \"%s\"",
             ts1, ts2, ts3, output);

    if (system(cmd_concat) != 0) {
        fprintf(stderr, "[FFMPEG_WRAPPER] Eroare la concatenare .ts\n");
        return -1;
    }

    // === 5. Cleanup fișiere temporare
    remove("tmp/part1.mp4");
    remove("tmp/part2.mp4");
    remove("tmp/part2_mod.mp4");
    remove("tmp/part3.mp4");
    remove(ts1);
    remove(ts2);
    remove(ts3);

    return 0;
}