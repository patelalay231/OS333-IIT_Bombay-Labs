#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10

pthread_mutex_t lock;
pthread_cond_t cond;
int turn = 0;

void* routine(void* arg){
    int index = *(int*)arg;
    free(arg);

    pthread_mutex_lock(&lock);
    while(index != turn){
        pthread_cond_wait(&cond, &lock);
    }
    printf("I am thread %d.\n", index);
    turn++;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(){
    pthread_t th[N];
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    for(int i=0;i<N;i++){
        int* index = malloc(sizeof(int));
        *index = i;
        if(pthread_create(&th[i],NULL,routine,index) != 0){
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }    

    for(int i=0; i<N; i++){
        if(pthread_join(th[i],NULL) != 0){
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return 0;
}