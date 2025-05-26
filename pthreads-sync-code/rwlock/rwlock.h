#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

using namespace std;

struct read_write_lock {
    pthread_mutex_t lock;          // Mutex for protecting shared state
    pthread_cond_t readers_ok;     // Condition variable for waiting readers
    pthread_cond_t writers_ok;     // Condition variable for waiting writers
    int readers;                   // Number of active readers
    int writers_waiting;           // Number of waiting writers
    int writer_active;             // 1 if a writer is active, 0 otherwise
};

void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
