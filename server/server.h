#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 4096
#define CRLF "\r\n"
#define INDEX_FILENAME "index.html"
#define INT32_DIGITS 10

typedef struct {
    char *method;
    char *uri;
    char *version;
    char *host;
    char *user_agent;
} http_req_t;

typedef struct {
    struct {
        char *version;
        char *status;
        char *reason;
    } status_line;

    char *content_type;
    int content_length;
} http_res_t;

/* HTTP */
int parse_http_request(char *raw, http_req_t *req);
char *create_http_response(char **content, http_res_t *res);
int handle_client(int client_sock);

/* Utils */
void cleanup(char **ptr);
long get_fsize(FILE *fd);

#endif
