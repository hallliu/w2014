#ifndef PROBETABLE_H
#define PROBETABLE_H

#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>
#include "hashtables.h"

struct bucket_elem {
    Packet_t *pkt;
    int key;
    volatile int max_dist;
    pthread_mutex_t datalock; // This protects the key and the pkt
    pthread_mutex_t distlock; // This protects the max_dist field
};

struct probe_table {
    bool (*add)(struct hashtable *, int, Packet_t *);
    bool (*remove)(struct hashtable *, int);
    bool (*contains)(struct hashtable *, int);
    volatile int cap;

    pthread_rwlock_t whole_lock;
    struct bucket_elem *elems;
};

struct probe_table *create_probetable(int cap);
#endif
