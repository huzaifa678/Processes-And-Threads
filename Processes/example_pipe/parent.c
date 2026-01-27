// let's implement the parent process that reads a CSV file and sends its content to a child process via a pipe.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {        // argc should have two arguments
        fprintf(stderr, "Usage: %s <csv_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd[2]; // file descriptors for the pipe
    if (pipe(fd) == -1) {
        perror("failed to create pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork(); // forking the child process
    if (pid < 0) {      // error handling
        perror("failed to fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {                  
        close(fd[1]);                 // Close write end
        dup2(fd[0], STDIN_FILENO);    // Pipe → stdin
        close(fd[0]);

        execl("./child", "child", NULL);
        perror("failed to exec child");
        exit(EXIT_FAILURE);
    } else {
        close(fd[0]);  // Close read end

        int csv_fd = open(argv[1], O_RDONLY); // Open the CSV file
        if (csv_fd < 0) {
            perror("failed to open csv");
            exit(EXIT_FAILURE);
        }

        char buffer[1024];  // create buffer for file reading
        ssize_t bytes;
        while ((bytes = read(csv_fd, buffer, sizeof(buffer))) > 0) {    // read bytes from the file
            write(fd[1], buffer, bytes); // write bytes to the pipe
        }

        close(csv_fd);  // finally close the CSV file
        close(fd[1]);   // signal EOF to child
        wait(NULL);
    }

    return 0;
}