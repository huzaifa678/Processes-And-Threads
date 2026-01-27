#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    char line[1024];   // buffer for reading lines
    int row_count = 0;
    int is_header = 1;

    while (fgets(line, sizeof(line), stdin)) {  // read line from the pipe until EOF
        if (is_header) {
            is_header = 0;
            continue;   
        }

        for (int i = 0; line[i]; i++) {     // convert the lines in the pipe to uppercase
            line[i] = toupper(line[i]);
        }

        fputs(line, stdout);  // finally write the processed line to standard output
        row_count++;
    }

    fprintf(stderr, "\nProcessed rows: %d\n", row_count);  // print the number of processed rows to standard error
    return 0;
}
