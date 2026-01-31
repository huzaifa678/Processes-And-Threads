// now let's create a pipe between two processes where the first process reads from a file and the second process filters the output to only include alphabetic characters.
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <data.txt>\n", argv[0]);
        return 1;
    }

    int fd[2];  // file descriptors for the pipe
    if (pipe(fd) == -1) {
        perror("failed to create pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid > 0) {          // if it is the parent process 
        close(fd[0]);              
        dup2(fd[1], STDOUT_FILENO);   // redirect stdout to pipe write end by duplicating fd[1]
        close(fd[1]);                   // close original write end

        execlp("cat", "cat", argv[1], NULL);  // finally execute cat to read the file
        perror("execlp cat");
    } else {
        close(fd[1]);       // close write end in child         
        dup2(fd[0], STDIN_FILENO); // redirect stdin to pipe read end by duplicating fd[0]
        close(fd[0]);  // close original read end

        execlp("tr", "tr", "-cd", "a-zA-Z", NULL); // execute tr to filter alphabetic characters
        perror("execlp tr");
    }

    waitpid(pid, NULL, 0);  // wait for the child process to finish

    return 0;
}
