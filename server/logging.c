#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void log_message(const char* level, const char* format, ...) {
    FILE* f = fopen("logs/server.log", "a");
    if (!f) return;

    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(f, "[%s] [%s] ", timebuf, level);

    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    fprintf(f, "\n");
    fclose(f);
}
