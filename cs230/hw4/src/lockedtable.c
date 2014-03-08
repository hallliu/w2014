#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>

#include "serial_lists.h"
#include "lockedtable.h"

#define RESIZE_THRESH 4
bool locked_add(struct hashtable *_t, int key, Packet_t *pkt);
bool locked_remove(struct hashtable *_t, int key);
bool locked_contains(struct hashtable *_t, int key);
void inc_size(struct locked_table *tab);

struct locked_table *create_lockedtable(int cap) {
    struct locked_table *tab = malloc (sizeof(struct locked_table));
    tab->initial_cap = cap;
    tab->cap = cap;

    tab->buckets = create_serial_lists(cap);
    tab->locks = malloc (cap * sizeof(pthread_rwlock_t));
    for (int i = 0; i < cap; i++) {
        pthread_rwlock_init (tab->locks + i, NULL);
    }

    tab->add = locked_add;
    tab->remove = locked_remove;
    tab->contains = locked_contains;
    return tab;
}

bool locked_add(struct hashtable *_t, int key, Packet_t *pkt) {
    struct locked_table *tab = (struct locked_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    bool trigger_resize = false;
    pthread_rwlock_wrlock (tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_add (tab->buckets + bucket_ind, key, pkt);

    if (tab->buckets[bucket_ind].size > RESIZE_THRESH)
        trigger_resize = true;

    pthread_rwlock_unlock (tab->locks + lock_ind);

    if (trigger_resize)
        inc_size(tab);

    return succ;
}

bool locked_remove(struct hashtable *_t, int key) {
    struct locked_table *tab = (struct locked_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    pthread_rwlock_wrlock (tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_remove (tab->buckets + bucket_ind, key);

    pthread_rwlock_unlock (tab->locks + lock_ind);
    return succ;
}

bool locked_contains(struct hashtable *_t, int key) {
    struct locked_table *tab = (struct locked_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    pthread_rwlock_rdlock (tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_contains (tab->buckets + bucket_ind, key);

    pthread_rwlock_unlock (tab->locks + lock_ind);
    return succ;
}

void inc_size(struct locked_table *tab) {
    // First save the size so we can tell if someone else already resized
    int old_size = tab->cap;
    // Acq all the locks
    for (int i = 0; i < tab->initial_cap; i++) {
        pthread_rwlock_wrlock(tab->locks + i);
    }

    if (tab->cap != old_size)
        goto end;

    // Allocate a new bucket array
    struct serial_list *new_buckets = create_serial_lists(tab->cap * 2);
    // Go through and reposition all the old elements
    for (int i = 0; i < tab->cap; i++) {
        struct serial_list *this_bucket = tab->buckets + i;
        struct serial_list_elem *e = this_bucket->head;
        while (e) {
            int new_ind = e->key & (tab->cap * 2 - 1);
            s_add(new_buckets + new_ind, e->key, e->pkt);
            e = e->next;
        }
    }
    // Destroy the old bucket array and adjust the size.
    destroy_serial_lists (tab->buckets, tab->cap);
    tab->buckets = new_buckets;
    tab->cap *= 2;

end:
    for (int i = 0; i < tab->initial_cap; i++) {
        pthread_rwlock_unlock(tab->locks + i);
    }
    return;
}
