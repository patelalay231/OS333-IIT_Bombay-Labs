#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->readers_ok, NULL);
    pthread_cond_init(&rw->writers_ok, NULL);
    rw->readers = 0;
    rw->writers_waiting = 0;
    rw->writer_active = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
    pthread_mutex_lock(&rw->lock);
    while (rw->writer_active || rw->writers_waiting > 0) {
        pthread_cond_wait(&rw->readers_ok, &rw->lock);
    }
    rw->readers++;
    pthread_mutex_unlock(&rw->lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
    pthread_mutex_lock(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        pthread_cond_signal(&rw->writers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
    pthread_mutex_lock(&rw->lock);
    rw->writers_waiting++;
    while (rw->writer_active || rw->readers > 0) {
        pthread_cond_wait(&rw->writers_ok, &rw->lock);
    }
    rw->writers_waiting--;
    rw->writer_active = 1;
    pthread_mutex_unlock(&rw->lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
    pthread_mutex_lock(&rw->lock);
    rw->writer_active = 0;
    if (rw->writers_waiting > 0) {
        pthread_cond_signal(&rw->writers_ok);
    } else {
        pthread_cond_broadcast(&rw->readers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}
