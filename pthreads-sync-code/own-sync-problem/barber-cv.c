#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CUSTOMERS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t customer_arrived = PTHREAD_COND_INITIALIZER;
pthread_cond_t barber_ready = PTHREAD_COND_INITIALIZER;

int waiting_customers = 0;
int chairs = 3;

void *barber(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (waiting_customers == 0) {
            printf("Barber is sleeping\n");
            pthread_cond_wait(&customer_arrived, &mutex);
        }
        waiting_customers--;
        printf("Barber is cutting hair. Waiting customers: %d\n", waiting_customers);
        pthread_cond_signal(&barber_ready);
        pthread_mutex_unlock(&mutex);

        sleep(2); // Simulate haircut time
    }
    return NULL;
}

void *customer(void *arg) {
    int id = *(int *)arg;
    sleep(rand() % 3); // Simulate arrival time

    pthread_mutex_lock(&mutex);
    if (waiting_customers < chairs) {
        waiting_customers++;
        printf("Customer %d is waiting. Total waiting: %d\n", id, waiting_customers);
        pthread_cond_signal(&customer_arrived);
        pthread_cond_wait(&barber_ready, &mutex);
        printf("Customer %d is getting a haircut.\n", id);
    } else {
        printf("Customer %d left, no chairs available.\n", id);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t barber_thread;
    pthread_t customers[MAX_CUSTOMERS];
    int ids[MAX_CUSTOMERS];

    pthread_create(&barber_thread, NULL, barber, NULL);

    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        ids[i] = i;
        pthread_create(&customers[i], NULL, customer, &ids[i]);
    }

    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    return 0;
}
