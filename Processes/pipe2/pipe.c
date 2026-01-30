#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <data.txt>\n", argv[0]);
        return 1;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("failed to create pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid > 0) {
        close(fd[0]);              
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]);

        execlp("cat", "cat", argv[1], NULL);
        perror("execlp cat");
    } else {
        close(fd[1]);             
        dup2(fd[0], STDIN_FILENO); 
        close(fd[0]);

        execlp("tr", "tr", "-cd", "a-zA-Z", NULL);
        perror("execlp tr");
    }

    waitpid(pid, NULL, 0);

    return 0;
}
