#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>
#include "locks.h"

// Tuned backoff lock parameters
#ifdef TUNED_BACKOFF
#define BACKOFF_MIN 476
#define BACKOFF_MAX 3255
#endif

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000};
    nanosleep(&sleep_time, NULL);
}

static struct lock_t *create_TAS(void);
static struct lock_t *create_backoff(int *delays);
static struct lock_t *create_mutex(void);
static struct lock_t *create_noop(void);
static struct lock_t *create_Alock(int n_threads);
static struct lock_t *create_CLH(void);
static struct lock_t *create_MCS(void);

struct lock_t *create_lock(char *lock_type, void *lock_info) {
    if (!strcmp(lock_type, "TAS")) {
        return create_TAS();
    }

    if (!strcmp(lock_type, "backoff")) {
        return create_backoff((int *) lock_info);
    }

    if (!strcmp(lock_type, "Alock")) {
        int *threads = lock_info;
        return create_Alock(*threads);
    }

    if (!strcmp(lock_type, "CLH")) {
        return create_CLH();
    }

    if (!strcmp(lock_type, "MCS")) {
        return create_MCS();
    }

    if (!strcmp(lock_type, "mutex")) {
        return create_mutex();
    }

    if (!strcmp(lock_type, "noop")) {
        return create_noop();
    }
    return NULL;
}
//-------------------No-op lock-------------------------------------//
void lock_noop (struct lock_t *l __attribute__((unused))) {
    return;
}

bool try_lock_noop (struct lock_t *l __attribute__((unused))) {
    return true;
}

void unlock_noop (struct lock_t *l __attribute__((unused))) {
    return;
}

void destroy_noop (struct lock_t *l) {
    free(l);
}

static struct lock_t *create_noop(void) {
    struct lock_t *l = malloc (sizeof(struct lock_t));
    l->lock = lock_noop;
    l->try_lock = try_lock_noop;
    l->unlock = unlock_noop;
    l->destroy_lock = destroy_noop;
    return l;
}

//-------------------TAS lock---------------------------------------//
struct TAS_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile int locked;
};

void lock_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock *) _l;
    while (__sync_lock_test_and_set(&l->locked, 1));
}

bool try_lock_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock *) _l;
    if (!__sync_lock_test_and_set(&l->locked, 1)) {
        return true;
    }
    return false;
}

void unlock_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock *) _l;
    __sync_lock_release(&l->locked);
}

void destroy_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock *) _l;
    free (l);
}

static struct lock_t *create_TAS(void) {
    struct TAS_lock *l = malloc (sizeof(struct TAS_lock));
    l->lock = lock_TAS;
    l->unlock = unlock_TAS;
    l->try_lock = try_lock_TAS;
    l->destroy_lock = destroy_TAS;
    l->locked = 0;

    return (struct lock_t *) l;
}

//-------------------Exponential backoff lock-----------------------//

struct backoff_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    int min_delay;
    int max_delay;
    volatile int locked;
    volatile unsigned rand_seed;
};

void lock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock *) _l;
    int curr_delay = l->min_delay <= 0 ? 1 : l->min_delay;
    while (1) {
        while (l->locked);

        if (!__sync_lock_test_and_set(&l->locked, 1))
            return;
        unsigned rs = __sync_fetch_and_add(&l->rand_seed, 20);
        usleep (rand_r(&rs) % curr_delay);
        curr_delay *= 2;
        if (curr_delay > l->max_delay || curr_delay < 0)
            curr_delay = l->max_delay;
    }
    return;
}

bool try_lock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock *) _l;
    if (!__sync_lock_test_and_set(&l->locked, 1)) {
        return true;
    }
    return false;
}

void unlock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock *) _l;
    __sync_lock_release(&l->locked);
}

void destroy_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock *) _l;
    free (l);
}

static struct lock_t *create_backoff(int *delays) {
    struct backoff_lock *l = malloc (sizeof(struct backoff_lock));
    l->lock = lock_backoff;
    l->unlock = unlock_backoff;
    l->try_lock = try_lock_backoff;
    l->destroy_lock = destroy_backoff;

#ifndef TUNED_BACKOFF
    if (delays[0] <= 1)
        l->min_delay = 2;
    else
        l->min_delay = delays[0];

    if (delays[1] <= l->min_delay)
        l->max_delay = l->min_delay;
    else
        l->max_delay = delays[1];
#else
    l->min_delay = BACKOFF_MIN;
    l->max_delay = BACKOFF_MAX;
#endif

    l->locked = 0;
    l->rand_seed = time(0);

    return (struct lock_t *) l;
}

//-------------------Array lock-------------------------------------//
struct Alock_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct padded_flag *flags;
    volatile unsigned tail;
    pthread_key_t slots;
    unsigned size; 
};

struct padded_flag {
    volatile int unlocked;
    char padding[64-sizeof(int)];
} __attribute__((packed));


void lock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;

    unsigned slot = __sync_fetch_and_add(&l->tail, 1);
    slot = slot & (l->size - 1);

    while (!l->flags[slot].unlocked);

    unsigned *my_slot = pthread_getspecific (l->slots);
    if (my_slot == NULL) {
        my_slot = malloc (sizeof(int)); // This might create a memory leak
        pthread_setspecific(l->slots, my_slot);
    }
    *my_slot = slot;
    return;
}

bool try_lock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    unsigned cur_tail = l->tail;
    cur_tail = cur_tail & (l->size - 1);

    if (!l->flags[cur_tail].unlocked)
        return false;
    lock_Alock(_l);
    return true;
}

void unlock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    unsigned *_slot = pthread_getspecific (l->slots);
    unsigned slot = *(_slot);
    unsigned next_slot = (slot == l->size - 1) ? 0 : slot + 1;
    
    l->flags[slot].unlocked = false;
    l->flags[next_slot].unlocked = true;
    return;
}

void destroy_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    free ((void *) l->flags);
    pthread_key_delete(l->slots);
    free (l);
}

static struct lock_t *create_Alock(int n_threads) {
    struct Alock_lock *l = malloc (sizeof(struct Alock_lock));
    l->lock = lock_Alock;
    l->unlock = unlock_Alock;
    l->try_lock = try_lock_Alock;
    l->destroy_lock = destroy_Alock;

    l->size = 1 << ((unsigned) log2((double) n_threads) + 1);

    int k __attribute__((unused)) = posix_memalign ((void **) &l->flags, 64, sizeof(struct padded_flag) * l->size);
    memset ((void *) l->flags, 0, sizeof(struct padded_flag) * l->size);
    l->flags->unlocked = true;
    l->tail = 0;
    pthread_key_create(&l->slots, free);

    return (struct lock_t *) l;
}

//-------------------CLH Queue lock---------------------------------//
struct CLH_queue_node {
    volatile int locked;
    char padding[64-sizeof(int)];
} __attribute__((packed));

struct CLH_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct CLH_queue_node *tail;
    pthread_key_t nodes;
    pthread_key_t prevs;
};

void lock_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    struct CLH_queue_node *node = pthread_getspecific(l->nodes);
    if (node == NULL) {
        int k __attribute__((unused)) = posix_memalign ((void **) &node, 64, sizeof(struct CLH_queue_node));
        pthread_setspecific(l->nodes, node);
    }

    node->locked = true;
    struct CLH_queue_node *prev = (struct CLH_queue_node *) __sync_lock_test_and_set(&l->tail, node);
    pthread_setspecific (l->prevs, prev);

    volatile int *locked = &prev->locked;
    while (*locked);
}

bool try_lock_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    if (l->tail->locked)
        return false;
    lock_CLH (_l);
    return true;
}

void unlock_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    struct CLH_queue_node *node = pthread_getspecific (l->nodes);
    struct CLH_queue_node *prev_node = pthread_getspecific (l->prevs);

    node->locked = false;
    pthread_setspecific(l->nodes, prev_node);
    return;
}

void destroy_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    pthread_key_delete (l->nodes);
    pthread_key_delete (l->prevs);
    free (l);
}

static struct lock_t *create_CLH(void) {
    struct CLH_lock *l = malloc (sizeof(struct CLH_lock));
    l->lock = lock_CLH;
    l->unlock = unlock_CLH;
    l->try_lock = try_lock_CLH;
    l->destroy_lock = destroy_CLH;

    int k __attribute__((unused)) = posix_memalign ((void **) &l->tail, 64, sizeof(struct CLH_queue_node));
    l->tail->locked = 0;

    pthread_key_create (&l->nodes, free);
    pthread_key_create (&l->prevs, NULL);

    return (struct lock_t *) l;
}

//-------------------MCS lock---------------------------------------//
struct MCS_queue_node {
    volatile int locked;
    struct MCS_queue_node *next;
    char padding[64-sizeof(int)-sizeof(void *)];
} __attribute__((packed));

struct MCS_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct MCS_queue_node *tail;
    pthread_key_t nodes;
};

void lock_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    struct MCS_queue_node *node = pthread_getspecific(l->nodes);
    if (node == NULL) {
        int k __attribute__((unused)) = posix_memalign((void **) &node, 64, sizeof(struct MCS_queue_node));
        node->locked = 0;
        node->next = NULL;
        pthread_setspecific(l->nodes, node);
    }
    
    struct MCS_queue_node *prev_node = (struct MCS_queue_node *) __sync_lock_test_and_set(&l->tail, node);
    if (prev_node) {
        node->locked = 1;
        prev_node->next = node;
        while (node->locked);
    }
    return;
}

bool try_lock_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    if (l->tail)
        return false;
    lock_MCS (_l);
    return true;
}

void unlock_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    volatile struct MCS_queue_node *node = pthread_getspecific (l->nodes);
    if (node->next == NULL) {
        if (__sync_bool_compare_and_swap(&l->tail, node, NULL))
            return;
        while (!node->next);
    }
    node->next->locked = 0;
    node->next = NULL;
}

void destroy_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    pthread_key_delete (l->nodes);
    free (l);
}

static struct lock_t *create_MCS(void) {
    struct MCS_lock *l = malloc (sizeof(struct MCS_lock));
    l->lock = lock_MCS;
    l->unlock = unlock_MCS;
    l->try_lock = try_lock_MCS;
    l->destroy_lock = destroy_MCS;

    l->tail = NULL;
    pthread_key_create (&l->nodes, free);

    return (struct lock_t *) l;
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
