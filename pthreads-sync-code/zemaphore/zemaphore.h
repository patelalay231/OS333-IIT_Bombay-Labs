#include <pthread.h>

typedef struct zemaphore {
    int count;          // Semaphore count
    pthread_mutex_t mutex; // Mutex for protecting the count
    pthread_cond_t cond;   // Condition variable for signaling
} zem_t;

void zem_init(zem_t *, int);
void zem_up(zem_t *);
void zem_down(zem_t *);
