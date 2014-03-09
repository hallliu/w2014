#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>

#include "probetable.h"

#define DIST_THRESH 4

bool probe_add(struct hashtable *_t, int key, Packet_t *pkt);
bool probe_remove(struct hashtable *_t, int key);
bool probe_contains(struct hashtable *_t, int key);
void probe_inc_size(struct probe_table *tab);

struct probe_table *create_probetable(int cap) {
    struct probe_table *tab = malloc (sizeof(struct probe_table));
    tab->cap = cap;

    tab->elems = calloc(cap, sizeof(struct bucket_elem));

    for (int i = 0; i < cap; i++) {
        pthread_mutex_init (&tab->elems[i].datalock, NULL);
        pthread_mutex_init (&tab->elems[i].distlock, NULL);
    }
    pthread_rwlock_init (&tab->whole_lock, NULL);

    tab->add = probe_add;
    tab->remove = probe_remove;
    tab->contains = probe_contains;
    return tab;
}

// To add, acquire the whole-table lock, then acquire the distlock
// for the home address. Then start walking from the home address until
// we find a empty place, acquiring a datalock at each. This way, each
// thread has at most one lock held of each type at any moment, preventing
// deadlock.
bool probe_add(struct hashtable *_t, int key, Packet_t *pkt) {
    struct probe_table *tab = (struct probe_table *) _t;

    pthread_rwlock_rdlock (&tab->whole_lock);

    int start_ind = key & (tab->cap - 1);
    struct bucket_elem *start_elem = tab->elems + start_ind;
    
    pthread_mutex_lock (&start_elem->distlock);
    
    int num_steps = 0;
    while (num_steps <= DIST_THRESH) {
        int next_ind = (start_ind + num_steps) & (tab->cap - 1);
        struct bucket_elem *e = tab->elems + next_ind;
        
        pthread_mutex_lock (&e->datalock);
        if (e->pkt == NULL) {
            e->pkt = pkt;
            e->key = key;
            pthread_mutex_unlock (&e->datalock); // Unlock here because we're done with modifying e
            start_elem->max_dist = num_steps > start_elem->max_dist ? num_steps : start_elem->max_dist;
            pthread_mutex_unlock (&start_elem->distlock);
            pthread_rwlock_unlock (&tab->whole_lock);
            return true;
        }
        if (e->key == key) {
            pthread_mutex_unlock (&e->datalock);
            pthread_mutex_unlock (&start_elem->distlock);
            pthread_rwlock_unlock (&tab->whole_lock);
            return false;
        }
        pthread_mutex_unlock (&e->datalock);
        num_steps++;
    }

    // At this point, we've failed to find a place. This means drop all our locks,
    // resize the table, and try again.
    pthread_mutex_unlock (&start_elem->distlock);
    pthread_rwlock_unlock (&tab->whole_lock);
    probe_inc_size (tab);
    return probe_add (_t, key, pkt);
}

bool probe_remove(struct hashtable *_t, int key) {
    struct probe_table *tab = (struct probe_table *) _t;
    pthread_rwlock_rdlock (&tab->whole_lock);
    int start_ind = key & (tab->cap - 1);
    struct bucket_elem *start_elem = tab->elems + start_ind;

    int num_steps = 0;
    while (num_steps <= start_elem->max_dist) {
        int next_ind = (start_ind + num_steps) & (tab->cap - 1);
        struct bucket_elem *e = tab->elems + next_ind;

        pthread_mutex_lock (&e->datalock);
        if (e->key == key) {
            e->pkt = NULL;
            e->key = -1;
            pthread_mutex_unlock (&e->datalock);
            pthread_rwlock_unlock (&tab->whole_lock);
            return true;
        }
        pthread_mutex_unlock (&e->datalock);
        num_steps++;
    }
    pthread_rwlock_unlock (&tab->whole_lock);
    return false;
}

bool probe_contains(struct hashtable *_t, int key) {
    struct probe_table *tab = (struct probe_table *) _t;
    pthread_rwlock_rdlock (&tab->whole_lock);
    int start_ind = key & (tab->cap - 1);
    struct bucket_elem *start_elem = tab->elems + start_ind;

    int num_steps = 0;
    while (num_steps <= start_elem->max_dist) {
        int next_ind = (start_ind + num_steps) & (tab->cap - 1);
        struct bucket_elem *e = tab->elems + next_ind;

        if (e->key == key) {
            pthread_rwlock_unlock (&tab->whole_lock);
            return true;
        }
        num_steps++;
    }
    pthread_rwlock_unlock (&tab->whole_lock);
    return false;
}

// Helper function for resize -- adds elements in without acquiring any locks. 
// Returns 0 on success, 1 if dist threshold gets exceeded, and 2 if it finds
// a duplicate (2 should never happen).
int lockless_add (struct bucket_elem *elems, int cap, int key, Packet_t *pkt) {
    int start_ind = key & (cap - 1);
    struct bucket_elem *start_elem = elems + start_ind;

    int num_steps = 0;
    while (num_steps <= DIST_THRESH) {
        int next_ind = (start_ind + num_steps) & (cap - 1);
        struct bucket_elem *e = elems + next_ind;
        
        if (e->pkt == NULL) {
            e->pkt = pkt;
            e->key = key;
            start_elem->max_dist = num_steps > start_elem->max_dist ? num_steps : start_elem->max_dist;
            return 0;
        }
        if (e->key == key) {
            return 2;
        }
        num_steps++;
    }

    // At this point, we've failed to find a place. This means we need to tell
    // resize to make a bigger one.
    return 1;
}


void probe_inc_size(struct probe_table *tab) {
    // First save the size so we can tell if someone else already resized
    int old_size = tab->cap;

    // Past this point, we are guaranteed completely exclusive access
    // to the whole table. 
    pthread_rwlock_wrlock (&tab->whole_lock);

    if (tab->cap != old_size) {
        pthread_rwlock_unlock (&tab->whole_lock);
        return;
    }

    bool success = true;
    int new_cap = tab->cap * 2;
    struct bucket_elem *new_elems;

    // Loop until the step number is below threshold again.
    do {
        success = true;
        // Alloc a new array with double the size and initialize the fields.
        new_elems = calloc(new_cap, sizeof(struct bucket_elem));


        // Loop through the old array and add them all in.
        for (int i = 0; i < tab->cap; i++) {
            struct bucket_elem *e = tab->elems + i;
            if (e->pkt == NULL) {
                continue;
            }
            int result = lockless_add(new_elems, new_cap, e->key, e->pkt);
            // The error case -- just notify of error and exit.
            if (result == 2) {
                printf("Uhoh -- resize encountered duplicate. not good.\n");
                exit(-1);
            }
            // The insufficient size case -- re-try the whole thing over again.
            if (result == 1) {
                success = false;
                new_cap *= 2;
                free (new_elems);
                break;
            }
        }
    } while (!success);
    
    // At this point, success is presumed. Thus we initialize the locks on the new elem structure
    // and clean up the old one.
    for (int i = 0; i < new_cap; i++) {
        pthread_mutex_init (&new_elems[i].datalock, NULL);
        pthread_mutex_init (&new_elems[i].distlock, NULL);
    }

    for (int i = 0; i < tab->cap; i++) {
        pthread_mutex_destroy (&tab->elems[i].datalock);
        pthread_mutex_destroy (&tab->elems[i].distlock);
    }
    free (tab->elems);
    tab->elems = new_elems;
    tab->cap = new_cap;
    pthread_rwlock_unlock (&tab->whole_lock);
    return;
}
