#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int item_to_produce = 0, curr_buf_size = 0;
int total_items, max_buf_size, num_workers, num_masters;

int *buffer;

// Synchronization variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void print_produced(int num, int master) {
    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
    printf("Consumed %d by worker %d\n", num, worker);
}

void *generate_requests_loop(void *data) {
    int thread_id = *(int *)data;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (item_to_produce >= total_items) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Wait if buffer is full
        while (curr_buf_size >= max_buf_size) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Produce item
        buffer[curr_buf_size] = item_to_produce;
        print_produced(item_to_produce, thread_id);
        curr_buf_size++;
        item_to_produce++;

        // Signal consumers
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *consume_request(void *data) {
    int thread_id = *(int *)data;

    while (1) {
        pthread_mutex_lock(&mutex);

        // If nothing to consume and production is done, exit
        if (curr_buf_size == 0 && item_to_produce >= total_items) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Wait if buffer is empty
        while (curr_buf_size == 0 && item_to_produce < total_items) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Consume item if available
        if (curr_buf_size > 0) {
            curr_buf_size--; // LIFO: consume last inserted
            int item = buffer[curr_buf_size];
            print_consumed(item, thread_id);
            pthread_cond_signal(&not_full);
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #num_masters\n");
        exit(1);
    }

    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
    num_workers = atoi(argv[3]);
    num_masters = atoi(argv[4]);

    buffer = (int *)malloc(sizeof(int) * max_buf_size);

    // Create producer threads
    pthread_t *master_thread = malloc(sizeof(pthread_t) * num_masters);
    int *master_thread_id = malloc(sizeof(int) * num_masters);

    for (int i = 0; i < num_masters; i++) {
        master_thread_id[i] = i;
        pthread_create(&master_thread[i], NULL, generate_requests_loop, &master_thread_id[i]);
    }

    // Create consumer threads
    pthread_t *worker_thread = malloc(sizeof(pthread_t) * num_workers);
    int *worker_thread_id = malloc(sizeof(int) * num_workers);

    for (int i = 0; i < num_workers; i++) {
        worker_thread_id[i] = i;
        pthread_create(&worker_thread[i], NULL, consume_request, &worker_thread_id[i]);
    }

    // Join all threads
    for (int i = 0; i < num_masters; i++) {
        pthread_join(master_thread[i], NULL);
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_join(worker_thread[i], NULL);
    }

    // Cleanup
    free(buffer);
    free(master_thread);
    free(master_thread_id);
    free(worker_thread);
    free(worker_thread_id);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
