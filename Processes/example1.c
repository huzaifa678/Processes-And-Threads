// let's somewhat create a minimal bash shell kind of implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIM " \t\r\n"


char* read_line() {
    char* line = malloc(MAX_INPUT);  

    if(!line) {
        perror("error allocating line");
        exit(1);
    }

    if (fgets(line, MAX_INPUT, stdin) == NULL) {    // get the lines from the standard input
        free(line);
        return NULL;
    }

    return line;
}

char** parse_line_into_arguments(char* line) {
    char** args = malloc(MAX_ARGS * sizeof(char*));    // allocate for maximum args size
    char* token;
    int i = 0;

    if (!args) {
        perror("error allocating arguments");
        exit(1);
    }

    token = strtok(line, DELIM);      // converts the line into token by finding the string before the delimeter
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, DELIM);  // skipping the delimeter 
    }
    args[i] = NULL;
    return args;
}


void execute(char** args) {
    if (args[0] == NULL) return;

    if (strcmp(args[0], "exit") == 0) {  // execute the exit command
        exit(0);
    }

    pid_t pid = fork();   // create the child process to execute the command  

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {   // execute the command finally by replacing the memory space of the child process
                                             // with the new program in this case the command given by the user
            perror("execvp");
        }
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        wait(NULL);    // wait until the child executes the command
    }

}
int main(int argc, char *argv[]) {
    char *line;
    char **args;

    if (argc > 1) {
        execute(&argv[1]);
    }

    while (1) {
        printf("> ");
        line = read_line();
        if (!line) break;

        args = parse_line_into_arguments(line);
        execute(args);

        free(line);
        free(args);
    }

    return 0;
}