// Now let's implement the barrier synchronization using mutexes and condition variables
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int multiplier;
    char *arr;
    size_t len;
    pthread_mutex_t *mutex;

    //barrier fields
    pthread_mutex_t *barrier_mutex;
    pthread_cond_t *barrier_cond;
    int *count;
    int num_threads;
} ThreadArgs;

// barrier function
void barrier_wait(ThreadArgs *args) {
    pthread_mutex_lock(args->barrier_mutex);
    (*args->count)++;
    if (*args->count == args->num_threads) {
        *args->count = 0;  // reset for reuse
        pthread_cond_broadcast(args->barrier_cond);  // wake up all waiting threads
    } else {
        while (*args->count != 0) {
            pthread_cond_wait(args->barrier_cond, args->barrier_mutex); // wait until the condition is signaled
        }
    }
    pthread_mutex_unlock(args->barrier_mutex); // finally the running thread unlocks the mutex
}

void *thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    // each thread does some work
    for (size_t i = 0; i < args->len; i++) {
        pthread_mutex_lock(args->mutex);
        args->arr[i] *= args->multiplier;
        pthread_mutex_unlock(args->mutex);
    }

    printf("Thread finished initial work\n");

    // wait at the barrier
    barrier_wait(args);

    // continue work after barrier
    printf("Thread continues after barrier\n");

    return NULL;
}

int main(int argc, char *argv[]) {
    // let's initialize the number of threads using the integer array
    
    int NUM_THREADS = 3;
    char arr[5] = {1, 2, 3, 4, 5};
    pthread_t threads[NUM_THREADS];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutex initialization for critical section

    pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER; // this mutex is for synchronizing the barrier
    pthread_cond_t barrier_cond = PTHREAD_COND_INITIALIZER;  // condition variable for the barrier
    int count = 0;

    ThreadArgs args[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].multiplier = i + 2;
        args[i].arr = arr;
        args[i].len = 5;
        args[i].mutex = &mutex;
        args[i].barrier_mutex = &barrier_mutex;
        args[i].barrier_cond = &barrier_cond;
        args[i].count = &count;
        args[i].num_threads = NUM_THREADS;

        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    // wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final array: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");


    // finally destroy everything before the main thread exits
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&barrier_mutex);
    pthread_cond_destroy(&barrier_cond);

    return 0;
}
