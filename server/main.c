// let's implement the HTTP server for applying the threading concepts
#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "socket.h"
#include "signal_handler.h"
#include "server.h"
#include "cal.h"

#define WORKER_COUNT 4

void *client_thread(void *arg) {    // client thread which will handle the client request
    int client_sock = *(int *)arg;
    free(arg);
    handle_client(client_sock);
    return NULL;
}

int main(void) {
    setup_signal_handlers();   // signal handlers initialized for graceful shutdown when signaled

    task_queue_init(&g_queue);

    pthread_t workers[WORKER_COUNT];    // worker threads pools created and asynchronously detached for completing the tasks 
    for (int i = 0; i < WORKER_COUNT; i++) {
        pthread_create(&workers[i], NULL, worker_thread, NULL);
        pthread_detach(workers[i]); 
    }

    int server_sock = create_server_socket(8080);   // server socket created on port 8080
    printf("Listening on http://localhost:8080\n");

    while (server_running) {
        fd_set set;  // file descriptor set for select
        FD_ZERO(&set);  // initialize the set
        FD_SET(server_sock, &set); // add the server socket to the set

        struct timeval timeout = {1, 0};  // timeout of 1 second for select
        int ready = select(server_sock + 1, &set, NULL, NULL, &timeout); // select call to monitor the server socket for incoming connections
        if (ready > 0) {
            int *client = malloc(sizeof(int));   // allocating memory for the client socket fd
            *client = accept(server_sock, NULL, NULL); // accepting the client connection
            if (*client >= 0) {         // if client accepted successfully, create a thread to handle the client
                pthread_t tid;
                pthread_create(&tid, NULL, client_thread, client); // create the client thread
                pthread_detach(tid);  // detach the thread to free resources when done
            } else { 
                free(client);
            }
        }
    }

    close(server_sock);  // finally close the server socket on shutdown
    return 0;
}