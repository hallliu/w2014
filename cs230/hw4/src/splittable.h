#ifndef SPLITTABLE_H
#define SPLITTABLE_H

#include <stdbool.h>

#include <packetsource.h>
#include "lockfree_lists.h"
#include "hashtables.h"

struct split_table {
    bool (*add)(struct hashtable *, int, Packet_t *);
    bool (*remove)(struct hashtable *, int);
    bool (*contains)(struct hashtable *, int);

    // Capacities assumed to be powers of 2
    int cap;

    struct lockfree_list *buckets;
};

struct split_table *create_splittable(int cap);
void initialize_index (struct split_table *tab, int index);
#endif
