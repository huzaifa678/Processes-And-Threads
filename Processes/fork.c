#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc , char* argv[]) {

    //lets get started with forks where the parents process creates its child process each fork call creates 2^n processes
    //when the parent calls this system call the kernel creates its copy which is the child process but the child process has its own
    //address space and its own PID. The parent and the child forks which makes upto 2^n processes where n is the number of fork calls

    fork();
    fork();
    fork();
    fork();
    printf("hello world\n");

    return 0;

}