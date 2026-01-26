#include "signal.h"
#include <signal.h>
#include <stdio.h>

// setting to true for server to run initially
volatile int server_running = 1;

// implementing the handler function for termination signals for graceful shutdown
static void handle_signal(int sig) {
    (void)sig;
    server_running = 0;
    printf("\nShutting down server...\n");
}

// setting up the signal handlers for SIGINT and SIGTERM
void setup_signal_handlers(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
}