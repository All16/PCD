#ifndef FFMPEG_WRAPPER_H
#define FFMPEG_WRAPPER_H

int ffmpeg_cut(const char* input, const char* start, const char* end, const char* output);
int ffmpeg_extract_audio(const char* input, const char* output);
int ffmpeg_convert(const char* input, const char* format, const char* output);
int ffmpeg_concat(const char* input1, const char* input2, const char* output);
int ffmpeg_change_resolution(const char* input, const char* resolution, const char* output);
///int ffmpeg_get_duration(const char* input, char* duration, size_t size);


#endif
