#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include <packetsource.h>

#include "lockfree_lists.h"
#include "splittable.h"

#define RESIZE_THRESH 4
#define PREALLOC_COUNT 1<<18

#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (typeof(x))((unsigned long)(x) & (~0x1l))

static inline int log2(int x) {
    int result = 0;
    while (x >>= 1, x)
        result++;
    return result;
}

bool split_add(struct hashtable *_t, int key, Packet_t *pkt);
bool split_remove(struct hashtable *_t, int key);
bool split_contains(struct hashtable *_t, int key);
void lf_inc_size(struct split_table *tab);

struct split_table *create_splittable(int cap) {
    struct split_table *tab = malloc (sizeof(struct split_table));
    tab->cap = cap;

    tab->buckets = create_lockfree_lists(PREALLOC_COUNT);
    // Create a single sentinal node at zero.
    lf_add(tab->buckets, 0, 0);

    tab->add = split_add;
    tab->remove = split_remove;
    tab->contains = split_contains;
    return tab;
}

void initialize_index (struct split_table *tab, int index) {
    // If already initialized, do nothing.
    if (tab->buckets[index].head)
        return;

    // Get the parent index, make sure it's initialized.
    int parent_ind = index & ((1 << log2(index)) - 1);
    initialize_index (tab, parent_index);

    // Allocate a sentinal node
    struct lf_elem *new_sentinal = calloc(1, sizeof(struct lf_elem));
    new_sentinal->key = index;
    new_sentinal->rev_key = rev_bits (index);

    // A modified version of the lock-free add, since we need to know what address
    // the added node is at.
    while (1) {
        struct lf_elem *prev = NULL, *curr = NULL;
        lf_find (tab->buckets + parent_ind, new_sentinal->rev_key, &prev, &curr);
    
        // Because of the way we did things, prev should never be unset. 
        // The zero sentinal node was inserted at list creation and we are
        // guaranteed that the parent ind's rev_key should come before ours.
        assert(prev);
    
        // This is if some other thread managed to initialize this thing before us.
        // If this happens, just free alloc'd memory and return.
        if (curr && curr->rev_key == new_sentinal->rev_key) {
            free (new_sentinal);
            return;
        }
    
        // Set the new sentinal's next pointer to curr and attempt to swing the
        // pointer of prev around. If we fail, try again.
        new_sentinal->next = curr;
        if (__sync_bool_compare_and_swap(&prev->next, curr, new_sentinal))
            break;
    }
     
    // Now that the sentinal is in, alter the head pointer of the current index.
    // We can only have gotten to this point if the new_sentinal we allocated was 
    // the one inserted into the linked list, so we're fine just changing this without
    // synchronization. 
    tab->buckets[index].head = new_sentinal;
    return;
}


bool split_add(struct hashtable *_t, int key, Packet_t *pkt) {
    struct split_table *tab = (struct split_table *) _t;

    int bucket_ind = key & (tab->cap - 1);
    bool succ = lf_add (tab->buckets + bucket_ind, key, pkt);

    if (tab->buckets[bucket_ind].size > RESIZE_THRESH) {
        trigger_resize = true;
    }

    pthread_mutex_unlock (tab->locks + lock_ind);

    if (trigger_resize)
        lf_inc_size(tab);

    return succ;
}

bool split_remove(struct hashtable *_t, int key) {
    struct split_table *tab = (struct split_table *) _t;
    int lock_ind = key & (tab->initial_cap - 1);

    pthread_mutex_lock (tab->locks + lock_ind);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = lf_remove (tab->buckets + bucket_ind, key);

    pthread_mutex_unlock (tab->locks + lock_ind);
    return succ;
}

bool split_contains(struct hashtable *_t, int key) {
    struct split_table *tab = (struct split_table *) _t;
    // Get a pointer to the access count to the current bucketlist-state
    // Resize will alter the location of this when it resizes so that 
    // it can watch the old one dwindle down to zero as the old
    // containment calls expire.
    volatile int *access_cnt = tab->contain_access_cnt;
    __sync_fetch_and_add(access_cnt, 1);

    int bucket_ind = key & (tab->cap - 1);
    bool succ = lf_contains (tab->buckets + bucket_ind, key);

    __sync_fetch_and_add(access_cnt, -1);
    return succ;
}

void lf_inc_size(struct split_table *tab) {
    // First save the size so we can tell if someone else already resized
    int old_size = tab->cap;
    if (old_size >= PREALLOC_COUNT) {
        printf("Oops: exceeded prealloc\n");
        return;
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
        struct lockfree_list *this_bucket = tab->buckets + i;
        struct lf_elem *e = this_bucket->head;
        drops[i] = NULL;
        int move_ctr = 0;
        while (e) {
            int new_ind = e->key & (tab->cap * 2 - 1);
            if (new_ind != i) {
                if (!MARKOF(e->next))
                    lf_add(tab->buckets + new_ind, e->key, e->pkt);
                move_ctr++;
            }
            else {
                drops[i] = e;
            }
            e = REFOF(e->next);
        }
        this_bucket->size -= move_ctr;
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
        unsigned long mark = MARKOF(e->next);
        e->next = (struct lf_elem *) mark;
    }
    free (drops);

end:
    for (int i = 0; i < tab->initial_cap; i++) {
        pthread_mutex_unlock(tab->locks + i);
    }
    return;
}
