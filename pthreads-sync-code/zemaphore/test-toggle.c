#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 3
#define NUM_ITER 10

zem_t zem[NUM_THREADS];

void *justprint(void *data)
{
    int thread_id = *((int *)data);

    for (int i = 0; i < NUM_ITER; i++)
    {
        zem_down(&zem[thread_id]);  // Wait for turn

        printf("This is thread %d\n", thread_id);

        int next_thread = (thread_id + 1) % NUM_THREADS;
        zem_up(&zem[next_thread]);  // Signal next thread
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t mythreads[NUM_THREADS];
    int mythread_id[NUM_THREADS];

    // Initialize zemaphores
    for (int i = 0; i < NUM_THREADS; i++)
    {
        zem_init(&zem[i], 0);
    }
    zem_up(&zem[0]);  // Start with thread 0

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        mythread_id[i] = i;
        pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(mythreads[i], NULL);
    }

    return 0;
}
