#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    const char *fifo = "fib_pipe";
    int fd;

    // open FIFO for reading
    fd = open(fifo, O_RDONLY);
    if (fd == -1) {
        perror("failed to open fifo for reading");
        exit(EXIT_FAILURE);
    }

    long long fib;
    printf("Fibonacci sequence received:\n");
    while (read(fd, &fib, sizeof(fib)) > 0) {
        printf("%lld ", fib);
    }
    printf("\n");

    close(fd);
    return 0;
}
