#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <limits.h>
#include <linux/limits.h>
#include <errno.h>
#include <json-c/json.h>
#include <microhttpd.h>
#include <signal.h>

#include "../include/ffmpeg_wrapper.h"
#include "../include/job_queue.h"
#include "../include/rest_api.h"

#define INCOMING_FOLDER "../videos/incoming/"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + NAME_MAX + 1))
#define PORT 5000
#define POST_BUFFER_SIZE 2048

struct ConnectionInfo {
    char *buffer;
};

extern volatile sig_atomic_t running;

void* start_rest_server(void* arg) {
    printf("[REST] Fir REST monitorizează fișiere JSON...\n");

    int fd = inotify_init();
    if (fd < 0) {
        perror("[REST] Eroare inotify_init");
        pthread_exit(NULL);
    }

    int wd = inotify_add_watch(fd, INCOMING_FOLDER, IN_CREATE);
    if (wd == -1) {
        perror("[REST] Eroare inotify_add_watch");
        close(fd);
        pthread_exit(NULL);
    }

    char buffer[BUF_LEN];
    fd_set fds;
    struct timeval timeout;

    while (running) {
        pthread_testcancel();
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(fd + 1, &fds, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(fd, &fds)) {
            int length = read(fd, buffer, BUF_LEN);
            if (length < 0) {
                perror("[REST] Eroare la citire inotify");
                continue;
            }

            int i = 0;
            while (i < length) {
                struct inotify_event* event = (struct inotify_event*)&buffer[i];

                if (event->len && (event->mask & IN_CREATE) && strstr(event->name, ".json")) {
                    char filepath[512];
                    snprintf(filepath, sizeof(filepath), "%s%s", INCOMING_FOLDER, event->name);
                    printf("[REST] Detectat fișier JSON nou: %s\n", filepath);

                    sleep(1);
                    FILE* f = fopen(filepath, "r");
                    if (!f) {
                        perror("[REST] Eroare deschidere fișier JSON");
                        i += EVENT_SIZE + event->len;
                        continue;
                    }

                    fseek(f, 0, SEEK_END);
                    long fsize = ftell(f);
                    rewind(f);

                    char *json_buf = malloc(fsize + 1);
                    if (!json_buf) {
                        perror("[REST] Alocare memorie JSON");
                        fclose(f);
                        i += EVENT_SIZE + event->len;
                        continue;
                    }

                    size_t read_bytes = fread(json_buf, 1, fsize, f);
                    fclose(f);
                    json_buf[read_bytes] = '\0';

                    struct json_object* job_json = json_tokener_parse(json_buf);
                    free(json_buf);

                    if (!job_json) {
                        printf("[REST] Fișier JSON invalid: %s\n", filepath);
                        i += EVENT_SIZE + event->len;
                        continue;
                    }

                    Job job;
                    memset(&job, 0, sizeof(Job));

                    const char* cmd = json_object_get_string(json_object_object_get(job_json, "command"));
                    const char* input = json_object_get_string(json_object_object_get(job_json, "input_file"));
                    const char* args = json_object_get_string(json_object_object_get(job_json, "args"));
                    const char* output = json_object_get_string(json_object_object_get(job_json, "output_file"));

                    snprintf(job.command, sizeof(job.command), "%s", cmd);
                    snprintf(job.input_file, sizeof(job.input_file), "videos/incoming/%s", input);
                    snprintf(job.args, sizeof(job.args), "%s", args);
                    snprintf(job.output_file, sizeof(job.output_file), "videos/outgoing/%s", output);
                    job.client_socket = -1;

                    job_queue_enqueue(job);
                    printf("[REST] Job adăugat în coadă din JSON: %s\n", job.command);
                    json_object_put(job_json);
                }

                i += EVENT_SIZE + event->len;
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    printf("[REST] Fir REST oprit.\n");
    pthread_exit(NULL);
}

enum MHD_Result handle_request(void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls)
{
    if (strcmp(method, "GET") == 0 && strcmp(url, "/") == 0) {
        const char *msg = "V-Edit REST server activ.\n";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(msg), (void *)msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    if (*con_cls == NULL) {
        struct ConnectionInfo *info = malloc(sizeof(struct ConnectionInfo));
        info->buffer = calloc(1, POST_BUFFER_SIZE);
        *con_cls = info;
        return MHD_YES;
    }

    struct ConnectionInfo *info = *con_cls;

    if (*upload_data_size != 0) {
        strncat(info->buffer, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    struct json_object *json = json_tokener_parse(info->buffer);
    printf("[DEBUG] JSON primit: %s\n", info->buffer);

    if (!json) {
        const char *err = "{\"error\": \"Invalid JSON\"}";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        free(info->buffer);
        free(info);
        *con_cls = NULL;
        return ret;
    }

    const char *filename = json_object_get_string(json_object_object_get(json, "filename"));
    const char *folder = json_object_get_string(json_object_object_get(json, "folder"));

    Job job;
    memset(&job, 0, sizeof(Job));
    job.client_socket = -1;

    // ---------------- CONCAT ----------------
    if (strcmp(method, "POST") == 0 && strcmp(url, "/concat") == 0) {
        const char *file1 = json_object_get_string(json_object_object_get(json, "file1"));
        const char *file2 = json_object_get_string(json_object_object_get(json, "file2"));
        if (!file1 || !file2 || !folder) goto missing;

        snprintf(job.command, sizeof(job.command), "concat");
        snprintf(job.args, sizeof(job.args), "%s %s", file1, file2);
        snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, file1);
        snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s", folder, file1);
    }

    // ---------------- CUT ----------------
    else if (strcmp(url, "/cut") == 0) {
        const char *start = json_object_get_string(json_object_object_get(json, "start"));
        const char *end = json_object_get_string(json_object_object_get(json, "end"));
        if (!filename || !folder || !start || !end) goto missing;

        snprintf(job.command, sizeof(job.command), "cut");
        snprintf(job.args, sizeof(job.args), "%s %s", start, end);
        snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, filename);
        snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s", folder, filename);
    }

    // ---------------- EXTRACT AUDIO ----------------
    else if (strcmp(url, "/extract_audio") == 0) {
        if (!filename || !folder) goto missing;

        snprintf(job.command, sizeof(job.command), "extract_audio");

        char base[128];
        strncpy(base, filename, sizeof(base));
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, filename);
        snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s.mp3", folder, base);
    }

    // ---------------- CHANGE RESOLUTION ----------------
    else if (strcmp(url, "/change_resolution") == 0) {
        const char *resolution = json_object_get_string(json_object_object_get(json, "resolution"));
        if (!filename || !folder || !resolution) goto missing;

        snprintf(job.command, sizeof(job.command), "change_resolution");
        snprintf(job.args, sizeof(job.args), "%s", resolution);
        snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, filename);
        snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s", folder, filename);
    }

    // ---------------- CUT EXCEPT ----------------
    else if (strcmp(url, "/cut_except") == 0) {
        const char *start = json_object_get_string(json_object_object_get(json, "start"));
        const char *end = json_object_get_string(json_object_object_get(json, "end"));
        if (!filename || !folder || !start || !end) goto missing;

        snprintf(job.command, sizeof(job.command), "cut_except");
        snprintf(job.args, sizeof(job.args), "%s %s", start, end);
        snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, filename);
        snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s", folder, filename);
    }

    // ---------------- SPEED SEGMENT ----------------
    else if (strcmp(method, "POST") == 0 && strcmp(url, "/speed_segment") == 0) {
    const char *start = json_object_get_string(json_object_object_get(json, "start"));
    const char *end = json_object_get_string(json_object_object_get(json, "end"));
    const char *factor = json_object_get_string(json_object_object_get(json, "factor"));

    if (!filename || !folder || !start || !end || !factor) {
        json_object_put(json);
        const char *err = "{\"error\": \"Missing parameters for speed_segment\"}";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        free(info->buffer); free(info); *con_cls = NULL;
        return ret;
    }

    snprintf(job.command, sizeof(job.command), "speed_segment");
    snprintf(job.args, sizeof(job.args), "%s %s %s", start, end, factor);
    snprintf(job.input_file, sizeof(job.input_file), "videos/%s/%s", folder, filename);
    snprintf(job.output_file, sizeof(job.output_file), "videos/%s/%s", folder, filename);
    }


    // ---------------- ENDPOINT NECUNOSCUT ----------------
    else {
        json_object_put(json);
        const char *err = "{\"error\": \"Unknown endpoint\"}";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, resp);
        MHD_destroy_response(resp);
        free(info->buffer); free(info); *con_cls = NULL;
        return ret;
    }

    // === Adaugăm job-ul în coadă ===
    job_queue_enqueue(job);
    json_object_put(json);

    const char *ok = "{\"status\": \"accepted\"}";
    struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(ok), (void *)ok, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    free(info->buffer); free(info); *con_cls = NULL;
    return ret;

missing: {
    json_object_put(json);
    const char *err = "{\"error\": \"Missing parameters\"}";
    struct MHD_Response *missing_resp = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_PERSISTENT);
    int missing_ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, missing_resp);
    MHD_destroy_response(missing_resp);
    free(info->buffer); free(info); *con_cls = NULL;
    return missing_ret;
}

}


