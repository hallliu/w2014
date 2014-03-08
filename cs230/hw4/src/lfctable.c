#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <packetsource.h>

#include "lockfree_lists.h"
#include "lfctable.h"

#define RESIZE_THRESH 4
#define PREALLOC_COUNT 1<<18

#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (typeof(x))((unsigned long)(x) & (~0x1l))

bool lfc_add(struct hashtable *_t, int key, Packet_t *pkt);
bool lfc_remove(struct hashtable *_t, int key);
bool lfc_contains(struct hashtable *_t, int key);
void inc_size(struct lfc_table *tab);

struct lfc_table *create_lfctable(int cap) {
    struct lfc_table *tab = malloc (sizeof(struct lfc_table));
    tab->initial_cap = cap;
    tab->cap = cap;

    tab->buckets = create_lockfree_lists(PREALLOC_COUNT);
    tab->locks = malloc (cap * sizeof(pthread_mutex_t));
    for (int i = 0; i < cap; i++) {
        pthread_mutex_init (tab->locks + i, NULL);
    }
    tab->contain_access_cnt = malloc(sizeof(int));
    *tab->contain_access_cnt = 0;

    tab->resize_needed = false;

    tab->add = lfc_add;
    tab->remove = lfc_remove;
    tab->contains = lfc_contains;
    return tab;
}

bool lfc_add(struct hashtable *_t, int key, Packet_t *pkt) {
    struct lfc_table *tab = (struct lfc_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    bool trigger_resize = false;
    pthread_mutex_lock(tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_add (tab->buckets + bucket_ind, key, pkt);

    if (tab->buckets[bucket_ind].size > RESIZE_THRESH) {
        trigger_resize = true;
    }

    pthread_mutex_unlock (tab->locks + lock_ind);

    if (trigger_resize)
        inc_size(tab);

    return succ;
}

bool lfc_remove(struct hashtable *_t, int key) {
    struct lfc_table *tab = (struct lfc_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    pthread_mutex_lock (tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_remove (tab->buckets + bucket_ind, key);

    pthread_mutex_unlock (tab->locks + lock_ind);
    return succ;
}

bool lfc_contains(struct hashtable *_t, int key) {
    struct lfc_table *tab = (struct lfc_table *) _t;
    // Get a pointer to the access count to the current bucketlist-state
    // Resize will alter the location of this when it resizes so that 
    // it can watch the old one dwindle down to zero as the old
    // containment calls expire.
    int *access_cnt = tab->contain_access_cnt;
    __sync_fetch_and_add(access_cnt, 1);

    int lock_ind = key & (tab->initial_cap - 1);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = s_contains (tab->buckets + bucket_ind, key);

    __sync_fetch_and_add(access_cnt, -1);
    return succ;
}

void inc_size(struct lfc_table *tab) {
    // First save the size so we can tell if someone else already resized
    int old_size = tab->cap;
    if (old_size >= PREALLOC_COUNT) {
        printf("Oops: exceeded prealloc\n");
        goto end;
    }

    // Acq all the locks
    for (int i = 0; i < tab->initial_cap; i++) {
        pthread_mutex_lock(tab->locks + i);
    }

    if (tab->cap != old_size)
        goto end;

    // Allocate an array of pointers to the last element that stays in place
    // Because of the recursive split ordering, the nodes that move and the nodes
    // that don't are precisely partitioned, so we can just drop the pointer to
    // the rest of the list to physically delete. 
    // If the entry is NULL, that means that everything got moved (unlikely).
    struct lf_elem **drops = malloc (tab->cap * sizeof(struct lf_elem *));

    // Go through and copy all the new elems to their new places
    for (int i = 0; i < tab->cap; i++) {
        struct serial_list *this_bucket = tab->buckets + i;
        struct serial_list_elem *e = this_bucket->head;
        drops[i] = NULL;
        while (e) {
            int new_ind = e->key & (tab->cap * 2 - 1);
            if (new_ind != i) {
                s_add(tab->buckets + new_ind, e->key, e->pkt);
            }
            else {
                drops[i] = e;
            }
            e = e->next;
        }
    }
    
    int *new_access_cnt = malloc(sizeof(int));
    *new_access_cnt = 0;
    volatile int *old_access_cnt = tab->contain_access_cnt;
    // Adjust the size, then swing the contain access counter over to the new one.
    tab->cap *= 2;
    __sync_synchronize(); //important to keep these in order
    tab->contain_access_cnt = new_access_cnt;

    // Spin waiting for the old access counter to go to zero
    while (*old_access_cnt);

    // Now we can go through the drop list and take out all the unnecessary elements.
    for (int i = 0; i < tab->cap / 2; i++) {
        struct lf_elem *e = drops[i];
        if (!e) {
            tab->buckets[i].head = NULL;
            continue;
        }
        int mark = MARKOF(e->next);
        e->next = mark;
    }
    free (drops);

end:
    for (int i = 0; i < tab->initial_cap; i++) {
        pthread_mutex_unlock(tab->locks + i);
    }
    return;
}
