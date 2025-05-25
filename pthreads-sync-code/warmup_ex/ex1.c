#include <stdio.h>
#include <pthread.h>

int global = 0;
pthread_mutex_t mutex;
void *increment_func(void * arg){
    for(int i=0; i<100000; i++){
        pthread_mutex_lock(&mutex);
        global++;
        pthread_mutex_unlock(&mutex);
    }
}

int main(){
    pthread_t th[10];
    pthread_mutex_init(&mutex, NULL);

    for(int i=0; i<10; i++){
        if(pthread_create(&th[i],NULL, &increment_func, NULL) != 0){
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0; i<10; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    printf("Final value of global: %d\n", global);
    pthread_mutex_destroy(&mutex);
    return 0;
}