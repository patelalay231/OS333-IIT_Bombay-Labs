#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* routine(void* arg){
    int index = *(int*)arg;
    printf("I am thread %d.\n",index);
    free(arg);
    return NULL;
}

int main(){
    pthread_t th[10];
    
    for(int i=0; i<10; i++){
        int *index = malloc(sizeof(int));
        *index = i;
        if(pthread_create(&th[i],NULL,&routine, index)!=0){
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
    printf("I am main thread.\n");
    return 0;
}