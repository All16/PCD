#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <microhttpd.h>
#include <json-c/json.h>
#include <signal.h>

#include "../include/job_queue.h" // Folosim coada ta de job-uri

#define PORT 5000
#define POST_BUFFER_SIZE 4096

// Variabile externe definite in main.c
extern volatile sig_atomic_t running;

// Structura pentru a gestiona datele primite prin POST de la libmicrohttpd
struct ConnectionInfo {
    char *buffer;
};

// =================================================================
// === SERVER API (cu libmicrohttpd) - Rol: Accepta job-uri      ===
// =================================================================

enum MHD_Result handle_api_request(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method, const char *version,
                                   const char *upload_data, size_t *upload_data_size, void **con_cls) {
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

    if (strcmp(method, "POST") != 0) {
        // Ignoram orice altceva in afara de POST
        free(info->buffer); free(info); *con_cls = NULL;
        return MHD_NO;
    }

    printf("[API] JSON primit: %s\n", info->buffer);
    struct json_object* json = json_tokener_parse(info->buffer);
    if (!json) {
        const char *err = "{\"error\": \"JSON invalid\"}";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_PERSISTENT);
        MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        free(info->buffer); free(info); *con_cls = NULL;
        return MHD_YES;
    }

    Job new_job;
    memset(&new_job, 0, sizeof(Job));

    // Extragem datele din JSON
    const char* filename = json_object_get_string(json_object_object_get(json, "filename"));
    // TODO: Extrage si ceilalti parametri (start, end, etc.) in functie de URL

    strncpy(new_job.command, url + 1, sizeof(new_job.command) - 1); // ex: "cut"
    if (filename) strncpy(new_job.input_file, filename, sizeof(new_job.input_file) - 1);
    // TODO: Populeaza new_job.args si new_job.output_file

    job_queue_enqueue(new_job);
    printf("[API] Job adaugat in coada: %s pentru %s\n", new_job.command, new_job.input_file);

    json_object_put(json);

    const char *ok_msg = "{\"status\": \"accepted\"}";
    struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(ok_msg), (void *)ok_msg, MHD_RESPMEM_PERSISTENT);
    MHD_queue_response(connection, MHD_HTTP_ACCEPTED, resp);
    MHD_destroy_response(resp);
    free(info->buffer); free(info); *con_cls = NULL;
    return MHD_YES;
}

void* start_api_server_thread(void* arg) {
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                 &handle_api_request, NULL, MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "[API][EROARE] Nu s-a putut porni serverul API.\n");
        return NULL;
    }
    printf("[API] Serverul API asculta pe portul %d\n", PORT);

    while (running) {
        sleep(1);
    }

    MHD_stop_daemon(daemon);
    printf("[API] Serverul API a fost oprit.\n");
    return NULL;
}

// =================================================================
// === WORKER THREAD - Rol: Executa job-urile din coada          ===
// =================================================================

void* worker_thread_func(void* arg) {
    printf("[WORKER] Worker-ul a pornit si monitorizeaza coada de job-uri.\n");

    while (running) {
        Job current_job;
        if (job_queue_dequeue(&current_job) == 0) { // 0 inseamna succes
            printf("[WORKER] Preluat job nou: %s pentru fisierul %s\n",
                   current_job.command, current_job.input_file);

            char command_str[1024];
            // TODO: Construieste comanda `ffmpeg` pe baza `current_job`
            // Exemplu:
            snprintf(command_str, sizeof(command_str), "ffmpeg -y -i videos/processing/%s -q:a 0 -map a videos/outgoing/%s.mp3",
                     current_job.input_file, current_job.input_file);

            printf("[WORKER] Execut: %s\n", command_str);
            system(command_str);

            printf("[WORKER] Job finalizat.\n");
        }
    }
    printf("[WORKER] Worker-ul a fost oprit.\n");
    return NULL;
}
