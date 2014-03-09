#ifndef LFCTABLE_H
#define LFCTABLE_H

#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>
#include "lockfree_lists.h"
#include "hashtables.h"

struct lfc_table {
    bool (*add)(struct hashtable *, int, Packet_t *);
    bool (*remove)(struct hashtable *, int);
    bool (*contains)(struct hashtable *, int);
    volatile int cap;

    // Capacities assumed to be powers of 2
    int initial_cap;

    pthread_mutex_t *locks;
    struct lockfree_list *buckets;
    volatile int *contain_access_cnt;

    volatile bool resize_needed;
};

struct lfc_table *create_lfctable(int cap);
#endif
