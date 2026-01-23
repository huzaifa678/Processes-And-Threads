// let's create a simple pipe between a parent and child process.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pipefd[2];
    pid_t cpid;
    char buf;

    printf("Creating pipe...\n");

    if (pipe(pipefd) == -1) {   // create the pipe
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    printf("Forking process...\n");
    cpid = fork();   // create the child process

    if (cpid == -1) {
        perror("failed for fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {    // Child process
        
        printf("Started (PID=%d)\n", getpid());
        printf("Closing write end of pipe\n");

        close(pipefd[1]);          // Close unused write end

        while (read(pipefd[0], &buf, 1) > 0) {   // read from the pipe
            printf("Read byte: '%c'\n", buf);
            write(STDOUT_FILENO, &buf, 1);       // write to standard output
        }

        printf("EOF detected, closing read end\n");
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);

    } else {   
        // Parent process
        printf("Started (PID=%d)\n", getpid());
        printf("Child PID=%d\n", cpid);
        printf("Closing read end of pipe\n");

        close(pipefd[0]);          // Close unused read end
        const char message[] = "Hello, Pipe!\n";
        printf("Writing message to pipe: \"%s\"\n", message);

        size_t len = sizeof(message) / sizeof(message[0]) - 1;
        printf("message length is %zu\n", len);

        write(pipefd[1], message, len);   // write to the pipe
        printf("Closing write end (sending EOF)\n");
        close(pipefd[1]);                 // Reader will see EOF

        printf("Waiting for child to finish...\n");
        wait(NULL);                       // Wait for child to finish
        exit(EXIT_SUCCESS);
    }
}