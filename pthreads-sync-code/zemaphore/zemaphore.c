#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"

void zem_init(zem_t *s, int value) {
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->cond, NULL);
    s->count = value;
}

void zem_down(zem_t *s) {
    pthread_mutex_lock(&s->mutex);

    if(s->count <= 0){
        pthread_cond_wait(&s->cond,&s->mutex);
    }
    s->count--;
    pthread_mutex_unlock(&s->mutex);
}

void zem_up(zem_t *s) {
    pthread_mutex_lock(&s->mutex);

    s->count++;
    pthread_cond_signal(&s->cond);
    
    pthread_mutex_unlock(&s->mutex);
}
