#ifndef COMMON_H
#define COMMON_H

#define MAX_FILENAME_LEN 256
#define MAX_COMMAND_LEN 64
#define MAX_ARGS_LEN 256
#define MAX_OUTPUT_LEN 256

typedef enum {
    CMD_CUT,
    CMD_CONVERT,
    CMD_EXTRACT_AUDIO,
    CMD_CUT_EXCEPT,
    CMD_CONCAT,
    CMD_CHANGE_RESOLUTION,
    CMD_SPEED_SEGMENT,
    CMD_UNKNOWN
} CommandType;

void log_message(const char* level, const char* format, ...);

#endif