#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {        // type struct to pass to the pthread as the argument before creating the thread
    int multiplier;
    int *arr;
    size_t len;
} ThreadArgs;


void *add_thread(void *arg) {
    ThreadArgs *args = arg;

    for (size_t i = 0; i < args->len; i++) {
        args->arr[i] += args->multiplier;
        printf("addition of elements %d\n", args->arr[i]);
    }

    return NULL;
}

int main() {
    int arr[] = {1, 2, 4, 8, 16, 32, 64, 128};

    pthread_t t1;

    ThreadArgs args = {
        .multiplier = 2,
        .arr = arr,
        .len = 8 
    };

    pthread_create(&t1, NULL, add_thread, &args);
    pthread_join(t1, NULL);
}