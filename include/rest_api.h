#ifndef REST_API_H
#define REST_API_H

#include <microhttpd.h>

// Handler REST HTTP (GET /, POST /cut)
enum MHD_Result handle_request(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls
);

#endif
