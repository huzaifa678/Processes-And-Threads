// now let's start by using the mutex to synchronize the threads keeping the critical section safe and the data consistent 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


typedef struct {         // a typical structure now adding the pointer to the mutex
    int multiplier;
    char *arr;
    size_t len;
    pthread_mutex_t *mutex;
} ThreadArgs;


void* update_array(void* args) {
    ThreadArgs* threadArgs = (ThreadArgs *)args;  // casting the void pointer to the structure's pointer 
    for (size_t i = 0; i < threadArgs->len; i++) {
        pthread_mutex_lock(threadArgs -> mutex);   // locking the mutex before entering the critical section
        threadArgs->arr[i] = (threadArgs->arr[i] - 'a' + threadArgs->multiplier) % 26 + 'a';  // tweaking ascii values for lowercase letters
        printf("Updated element %zu: %c\n", i, threadArgs -> arr[i]);
        pthread_mutex_unlock(threadArgs -> mutex);   // finally unlocking the mutex for the next thread to access the critical section
    }
    return NULL;
}

int main() {

    char arr[] = {'c', 'o', 'd', 'e', 's', 'y', 's', '\0'};  // in this case a character array for the data
    size_t len = sizeof(arr) - 1;  // excluding the null terminator

    pthread_t t1, t2;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);   // initialize the mutex firstly


    // defining two structure one for thread1 and the other for thread2

    ThreadArgs args1 = {
        .multiplier = 2,
        .arr = arr,
        .len = len,
        .mutex = &mutex
    };

    ThreadArgs args2 = {
        .multiplier = 4,
        .arr = arr,
        .len = len,
        .mutex = &mutex
    };

    pthread_create(&t1, NULL, update_array, &args1);  // creating the threads
    pthread_create(&t2, NULL, update_array, &args2);

    pthread_join(t1, NULL);      // thread1 joins first after the main thread
    pthread_join(t2, NULL);     // thread2 joins after thread1 finishes

    pthread_mutex_destroy(&mutex);  // finally destroy the mutex after threads finish their execution

    printf("\nFinal array: %s\n", arr);

    return 0;

}