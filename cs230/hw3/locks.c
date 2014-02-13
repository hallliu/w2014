#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "locks.h"

//static struct lock_t *create_TAS(void);
//static struct lock_t *create_backoff(double min_delay, double max_delay);
static struct lock_t *create_mutex(void);
//static struct lock_t *create_Alock(int max_threads);
//static struct lock_t *create_CLH(void);
//static struct lock_t *creat_MCS(void);

struct lock_t *create_lock(char *lock_type, void *lock_info) {
    if (!strcmp(lock_type, "mutex")) {
        return create_mutex();
    }
    return NULL;
}

//-------------------Pthreads mutex lock----------------------------//
struct mutex_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    pthread_mutex_t *mutex;
};

void lock_mutex (struct lock_t *_l) {
    struct mutex_lock *l = (struct mutex_lock *) _l;
    pthread_mutex_lock (l->mutex);
}

bool try_lock_mutex (struct lock_t *_l) {
    struct mutex_lock *l = (struct mutex_lock *) _l;
    if (pthread_mutex_trylock (l->mutex))
        return false;
    return true;
}

void unlock_mutex (struct lock_t *_l) {
    struct mutex_lock *l = (struct mutex_lock *) _l;
    pthread_mutex_unlock (l->mutex);
}

void destroy_mutex (struct lock_t *_l) {
    struct mutex_lock *l = (struct mutex_lock *) _l;
    pthread_mutex_destroy (l->mutex);
    free (l->mutex);
    free (l);
}

static struct lock_t *create_mutex(void) {
    pthread_mutex_t *mutex = malloc (sizeof(pthread_mutex_t));
    pthread_mutex_init (mutex, NULL);
    
    struct mutex_lock *l = malloc (sizeof(struct mutex_lock));
    l->lock = lock_mutex;
    l->unlock = unlock_mutex;
    l->try_lock = try_lock_mutex;
    l->destroy_lock = destroy_mutex;
    l->mutex = mutex;

    return (struct lock_t *) l;
}
