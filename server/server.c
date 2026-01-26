#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "cal.h"
#include <ctype.h>

void cleanup(char **ptr) { // clean up to finally clean the memory after usage
    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

void url_decode(char *dst, const char *src) {       // application logic for URL decoding
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}


int parse_http_request(char *raw, http_req_t *req) {    // parsing the raw HTTP request
    char *saveptr;
    char *line = strtok_r(raw, CRLF, &saveptr);  // converting the raw request into lines using CRLF as the delimeter
    if (!line) return -1;

    // tokenize first line
    char *method = strtok(line, " ");
    char *uri = strtok(NULL, " ");
    char *version = strtok(NULL, " ");

    if (!method || !uri || !version) return -1;

    // populating the request structure values by duplicating the strings
    req->method = strdup(method);
    req->uri = strdup(uri);
    req->version = strdup(version);

    // until all headers are parsed in the line extract Host and User-Agent headers and break on empty line
    while ((line = strtok_r(NULL, CRLF, &saveptr))) {
        if (strlen(line) == 0) break;

        if (strncmp(line, "Host:", 5) == 0) {
            req->host = strdup(line + 6);
        } else if (strncmp(line, "User-Agent:", 11) == 0) {
            req->user_agent = strdup(line + 12);
        }
    }
    return 0;
}

char *create_http_response(char **content, http_res_t *res) {
    // initlializing the response structure values with correct HTTP response values given the request was recieved
    res->status_line.version = "HTTP/1.1";
    res->status_line.status = "200";
    res->status_line.reason = "OK";
    res->content_type = "text/html";  // for sending the HTML page to the client 

    if (!content || !*content) {    // error handling on no content generation
        res->status_line.status = "500";
        res->status_line.reason = "Server Error";
        return NULL;
    }

    res->content_length = strlen(*content);

    char *res_str = calloc(res->content_length + 256, 1);  // allocating memory for the response string
    char lenbuf[INT32_DIGITS + 1];  // buffer for content length string

    sprintf(lenbuf, "%d", res->content_length);  // converting content length integer to string

    // constructing the response string step by step for the status line, headers and finally the content and it's type
    strcat(res_str, res->status_line.version);   
    strcat(res_str, " ");
    strcat(res_str, res->status_line.status);
    strcat(res_str, " ");
    strcat(res_str, res->status_line.reason);
    strcat(res_str, CRLF);

    strcat(res_str, "Content-Type: ");
    strcat(res_str, res->content_type);
    strcat(res_str, CRLF);

    strcat(res_str, "Content-Length: ");
    strcat(res_str, lenbuf);
    strcat(res_str, CRLF);
    strcat(res_str, CRLF);

    // finally appending the content to the response string
    strcat(res_str, *content);
    return res_str;
}

long get_fsize(FILE *fd) {
    fseek(fd, 0, SEEK_END);
    long size = ftell(fd);
    rewind(fd);
    return size;
}

int handle_client(int client_sock) {
    char buf[BUFFER_SIZE + 1];   // buffer for receiving the client request
    int bytes = recv(client_sock, buf, BUFFER_SIZE, 0);  // receiving the client request
    if (bytes <= 0) return -1;
  
    buf[bytes] = 0;  // null-terminate the received data
 
    http_req_t req = {0};  // initializing the request structure
    if (parse_http_request(buf, &req) != 0) return -1; // parsing it and error handling for failure

    if (strncmp(req.uri, "/calc?", 6) == 0) {       // if the request involves calculation task
        char *expr = strstr(req.uri, "expr=");      // finding the expr parameter in the URI

        // error handling for missing expr parameter
        if (!expr) {
            char *msg = strdup("Missing expr parameter!\n"); // creating error message
            http_res_t res = {0};  // initilializing again to 0
            char *response = create_http_response(&msg, &res); // creating the HTTP response for the error message
            send(client_sock, response, strlen(response), 0);  // sending to the client 
            // finally cleaning up the allocated memory
            cleanup(&msg); 
            cleanup(&response);
            goto cleanup_req;
        }
        expr += 5; // skip "expr="

        // URL decode
        char expr_decoded[MAX_EXPR] = {0};
        url_decode(expr_decoded, expr);

        // initializing task
        calc_task_t task = {0};

        // parse task type and parameters
        if (strncmp(expr_decoded, "derivative:", 11) == 0) {
            task.type = DERIVATIVE;
            strncpy(task.expr, expr_decoded + 11, MAX_EXPR - 1); // copying the expression after "derivative:"
        } else if (strncmp(expr_decoded, "integrate:", 10) == 0) {
            task.type = INTEGRAL;
            char *p = strchr(expr_decoded, ':'); // finding the colon after "integrate"
            if (p) p++;
            sscanf(p, "%[^,],%lf,%lf", task.expr, &task.a, &task.b); // parsing the expression and bounds
        } else {
            // another error handling for invalid task type
            char *msg = strdup("Invalid task type!\n");
            http_res_t res = {0};
            char *response = create_http_response(&msg, &res);
            send(client_sock, response, strlen(response), 0);
            cleanup(&msg);
            cleanup(&response);
            goto cleanup_req;
        }

        // enqueue task to global queue
        enqueue_task(&g_queue, &task);

        // responding immediately
        char *msg = strdup("Task queued successfully!\n");
        http_res_t res = {0};
        char *response = create_http_response(&msg, &res);
        send(client_sock, response, strlen(response), 0);

        // finallt cleaning up
        cleanup(&msg);
        cleanup(&response);

    cleanup_req:
        cleanup(&req.method);
        cleanup(&req.uri);
        cleanup(&req.version);
        cleanup(&req.host);
        cleanup(&req.user_agent);
        close(client_sock);
        return 0;
    }

    // serving HTML file when the client first connects to the server
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./html%s",
             strcmp(req.uri, "/") == 0 ? "/index.html" : req.uri); // determining the file path to serve

    FILE *fd = fopen(filepath, "r"); // opening the file for reading
    if (!fd) {                       // error handling for file not found    
        perror("failed to open the file");
        return -1;
    }

    long fsize = get_fsize(fd); // getting the file size 
    char *page = calloc(fsize + 1, 1); // allocating memory for the file content
    fread(page, 1, fsize, fd);  // reading the file content into the allocated memory
    fclose(fd);  // closing the file after reading
 
    http_res_t res = {0};
    char *response = create_http_response(&page, &res); // passing the content to create the HTTP response
    send(client_sock, response, strlen(response), 0); // sending to the client finally

    // finally cleaning up the allocated memory
    cleanup(&page);
    cleanup(&response);
    cleanup(&req.method);
    cleanup(&req.uri);
    cleanup(&req.version);
    cleanup(&req.host);
    cleanup(&req.user_agent);

    close(client_sock);
    return 0;
}
