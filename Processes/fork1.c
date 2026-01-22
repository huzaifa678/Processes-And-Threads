#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        //Child process
        printf("Hello from the Child process! PID: %d\n", getpid());
    } else {
        // Parent process
        printf("Hello from the Parent process! Child PID: %d, My PID: %d\n", pid, getpid());
    }

    return 0;
}
