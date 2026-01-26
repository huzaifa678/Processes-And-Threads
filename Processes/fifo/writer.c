// let's create a FIFO (named pipe) writer that generates Fibonacci numbers and writes them to the pipe
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    const char *fifo = "fib_pipe";
    int fd;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_fibonacci>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);  // number of Fibonacci numbers to generate
    if (n <= 0) {
        fprintf(stderr, "Please enter a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    // open FIFO for writing
    fd = open(fifo, O_WRONLY);
    if (fd == -1) {
        perror("failed to open fifo for writing");
        exit(EXIT_FAILURE);
    }

    long long a = 0, b = 1;
    printf("Writing Fibonacci sequence to pipe:\n");
    for (int i = 0; i < n; i++) {
        long long fib = (i == 0) ? 0 : (i == 1) ? 1 : a + b;
        if (i > 1) { a = b; b = fib; }

        // write the number to the pipe
        if (write(fd, &fib, sizeof(fib)) == -1) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // also print the number
        printf("%lld ", fib);
        fflush(stdout); // ensure it prints immediately
    }
    printf("\n");

    close(fd);
    return 0;
}


