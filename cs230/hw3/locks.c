#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "locks.h"

static struct lock_t *create_TAS(void);
static struct lock_t *create_backoff(int min_delay, int max_delay);
static struct lock_t *create_mutex(void);
static struct lock_t *create_noop(void);
static struct lock_t *create_Alock(int n_threads);
//static struct lock_t *create_CLH(void);
//static struct lock_t *creat_MCS(void);

struct lock_t *create_lock(char *lock_type, void *lock_info) {
    if (!strcmp(lock_type, "TAS")) {
        return create_TAS();
    }

    if (!strcmp(lock_type, "backoff")) {
        int *delays = lock_info
        return create_backoff(delays[0], delays[1]);
    }

    if (!strcmp(lock_type, "Alock")) {
        int *threads = lock_info;
        return create_Alock(*threads);
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
    struct TAS_lock *l = (struct TAS_lock) _l;
    while (__sync_lock_test_and_set(&l->locked, 1));
}

bool try_lock_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock) _l;
    if (!__sync_lock_test_and_set(&l->locked, 1)) {
        return true;
    }
    return false;
}

void unlock_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock) _l;
    __sync_lock_release(&l->locked);
}

void destroy_TAS (struct lock_t *_l) {
    struct TAS_lock *l = (struct TAS_lock) _l;
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
};

void lock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock *) _l;
    int curr_delay = l->min_delay;
    while (1) {
        while (l->locked);
        if (!__sync_lock_test_and_set(&l->locked, 1))
            return;
        usleep (rand() % curr_delay);
        curr_delay *= 2;
        if (curr_delay > l->max_delay)
            curr_delay = l->max_delay;
    }
    return;
}

bool try_lock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock) _l;
    if (!__sync_lock_test_and_set(&l->locked, 1)) {
        return true;
    }
    return false;
}

void unlock_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock) _l;
    __sync_lock_release(&l->locked);
}

void destroy_backoff (struct lock_t *_l) {
    struct backoff_lock *l = (struct backoff_lock) _l;
    free (l);
}

static struct lock_t *create_backoff(int min_delay, int max_delay) {
    struct backoff_lock *l = malloc (sizeof(struct backoff_lock));
    l->lock = lock_backoff;
    l->unlock = unlock_backoff;
    l->try_lock = try_lock_backoff;
    l->destroy_lock = destroy_backoff;
    l->min_delay = min_delay;
    l->max_delay = max_delay;
    l->locked = 0;

    return (struct lock_t *) l;
}

//-------------------Array lock-------------------------------------//
struct Alock_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct padded_flag *flags;
    volatile int tail;
    pthread_key_t slots;
    int size;
}

struct padded_flag {
    volatile int unlocked;
    char padding[64-sizeof(int)];
} __attribute__((packed, align(64)));


void lock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;

    __sync_bool_compare_and_swap(&l->tail, l->size, 0);
    int slot = __sync_fetch_and_add(&l->tail, 1);

    if (slot >= l->size)
        slot -= l->size;

    while (!l->flags[slot].unlocked);
    int *my_slot = pthread_getspecific (l->slots);
    if (my_slot == NULL) {
        my_slot = malloc (sizeof(int)); // This might create a memory leak
        pthread_setspecific(l->slots, my_slot);
    }
    *my_slot = slot;
    return;
}

bool try_lock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    if (!l->flags[l->tail].unlocked)
        return false;
    lock_Alock(_l);
    return true;
}

void unlock_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    int *_slot = pthread_getspecific (l->slots);
    int slot = *(_slot);
    int next_slot = (slot == l->size - 1) ? 0 : slot + 1;
    
    l->flags[slot].unlocked = true;
    l->flags[next_slot].unlocked = false;
    return;
}

void destroy_Alock (struct lock_t *_l) {
    struct Alock_lock *l = (struct Alock_lock *) _l;
    free (l->flags);
    pthread_key_destroy(l->slots);
    free (l);
}

static struct lock_t *create_Alock(int n_threads) {
    struct Alock_lock *l = malloc (sizeof(struct Alock_lock));
    l->lock = lock_Alock;
    l->unlock = unlock_Alock;
    l->try_lock = try_lock_Alock;
    l->destroy_lock = destroy_Alock;

    l->flags = aligned_alloc (64, sizeof(struct padded_flag) * n_threads);
    memset (l->flags, 0, sizeof(struct padded_flag) * n_threads);
    l->tail = 0;
    pthread_key_create(&l->slots, free);
    l->size = n_threads;

    return (struct lock_t *) l;
}

//-------------------CLH Queue lock---------------------------------//
struct CLH_queue_node {
    volatile int locked;
    struct CLH_queue_node *pred;
    char padding[64-sizeof(int)-sizeof(void *)];
} __attribute__((packed, align(64)));

struct CLH_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct CLH_queue_node *tail;
    pthread_key_t nodes;
}

void lock_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    struct CLH_queue_node *node = pthread_getspecific(l->nodes);
    if (node == NULL) {
        node = aligned_alloc(64, sizeof(struct CLH_queue_node));
        pthread_setspecific(l->nodes, node);
    }

    node->locked = true;
    node->pred = __sync_lock_test_and_set(&l->tail, node);
    while (node->pred->locked);
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
    struct CLH_queue_node *prev_node = node->pred;
    node->locked = 0;
    pthread_setspecific(l->nodes, prev_node);
    return;
}

void destroy_CLH (struct lock_t *_l) {
    struct CLH_lock *l = (struct CLH_lock *) _l;
    pthread_key_destroy (l->nodes);
    // This is supposed to guard against the nodes getting in a looped state. Start at 
    // the node after the tail and set the tail's pred to null so we stop next time it
    // comes around.
    volatile struct CLH_queue_node *curr = l->tail->pred;
    struct CLH_queue_node *pred;
    l->tail->pred = NULL;
    while (curr) {
        pred = curr->pred;
        free ((void *) curr);
        curr = pred;
    }
    free (l);
}

static struct lock_t *create_CLH(void) {
    struct CLH_lock *l = malloc (sizeof(struct CLH_lock));
    l->lock = lock_CLH;
    l->unlock = unlock_CLH;
    l->try_lock = try_lock_CLH;
    l->destroy_lock = destroy_CLH;

    l->tail = aligned_alloc(64, sizeof(struct CLH_queue_node));
    l->tail->locked = 0;
    l->tail->pred = NULL;

    pthread_key_create (&l->nodes, free);

    return (struct lock_t *) l;
}

//-------------------MCS lock---------------------------------------//
struct MCS_queue_node {
    volatile int locked;
    struct MCS_queue_node *next;
    char padding[64-sizeof(int)-sizeof(void *)];
} __attribute__((packed, align(64)));

struct MCS_lock {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
    volatile struct MCS_queue_node *tail;
    pthread_key_t nodes;
}

void lock_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    struct MCS_queue_node *node = pthread_getspecific(l->nodes);
    if (node == NULL) {
        node = aligned_alloc(64, sizeof(struct MCS_queue_node));
        node->locked = 0;
        node->next = NULL;
        pthread_setspecific(l->nodes, node);
    }
    
    struct MCS_queue_node *prev_node = __sync_lock_test_and_set(&l->tail, node);
    if (prev_node) {
        node->locked = 1;
        prev->next = node;
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
    struct MCS_queue_node *node = pthread_getspecific (l->nodes);
    if (node->next == NULL) {
        if (__sync_bool_compare_and_swap(&l->tail, node, NULL))
            return;
        while (!node->next);
    }
    node->next.locked = 0;
    node->next = NULL;
}

void destroy_MCS (struct lock_t *_l) {
    struct MCS_lock *l = (struct MCS_lock *) _l;
    pthread_key_destroy (l->nodes);
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
