#ifndef LOCKEDTABLE_H
#define LOCKEDTABLE_H

#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>
#include "serial_lists.h"
#include "hashtables.h"

struct locked_table {
    bool (*add)(struct hashtable *, int, Packet_t *);
    bool (*remove)(struct hashtable *, int);
    bool (*contains)(struct hashtable *, int);
    volatile int cap;

    // Capacities assumed to be powers of 2
    int initial_cap;

    pthread_rwlock_t *locks;
    struct serial_list *buckets;
};

struct locked_table *create_lockedtable(int cap);
#endif
