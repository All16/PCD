#ifndef FFMPEG_WRAPPER_H
#define FFMPEG_WRAPPER_H

/**
 * Taie un segment dintr-un fișier video.
 *
 * @param input Calea către fișierul video sursă.
 * @param start Timpul de început în format HH:MM:SS.
 * @param end   Timpul de sfârșit în format HH:MM:SS.
 * @param output Calea către fișierul video rezultat.
 * @return 0 dacă s-a executat cu succes, altfel un cod de eroare.
 */
int ffmpeg_cut(const char* input, const char* start, const char* end, const char* output);

/**
 * Extrage doar componenta audio dintr-un fișier video.
 *
 * @param input Calea către fișierul video.
 * @param output Calea către fișierul audio rezultat (.mp3).
 * @return 0 dacă s-a executat cu succes, altfel un cod de eroare.
 */
int ffmpeg_extract_audio(const char* input, const char* output);

/**
 * Convertește un fișier video într-un alt format (ex: mp4 -> avi).
 *
 * @param input Calea către fișierul sursă.
 * @param format Formatul țintă (ex: "avi").
 * @param output_base Numele de bază al fișierului de ieșire (fără extensie).
 * @return 0 dacă s-a executat cu succes, altfel un cod de eroare.
 */
int ffmpeg_convert(const char* input, const char* format, const char* output_base);

#endif // FFMPEG_WRAPPER_H