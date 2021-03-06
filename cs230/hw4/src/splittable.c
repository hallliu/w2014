#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <assert.h>

#include <packetsource.h>

#include "lockfree_lists.h"
#include "splittable.h"

#define RESIZE_THRESH 4
#define PREALLOC_COUNT 1<<22

#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (typeof(x))((unsigned long)(x) & (~0x1l))

static inline int blog2(int x) {
    int result = 0;
    while (x >>= 1, x)
        result++;
    return result;
}

bool split_add(struct hashtable *_t, int key, Packet_t *pkt);
bool split_remove(struct hashtable *_t, int key);
bool split_contains(struct hashtable *_t, int key);

struct split_table *create_splittable(int cap) {
    struct split_table *tab = malloc (sizeof(struct split_table));
    tab->cap = cap;

    tab->buckets = create_lockfree_lists(PREALLOC_COUNT);
    // Create a single sentinal node at zero.
    lf_add(tab->buckets, 0, 0, NULL);

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
    int parent_ind = index & ((1 << blog2(index)) - 1);
    initialize_index (tab, parent_ind);

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
        // assert(prev);
    
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
    int cap = tab->cap;
    int ind = key & (cap - 1);
    int num_steps;
    // Make sure index is initialized
    if (!tab->buckets[ind].head)
        initialize_index(tab, ind);

    // Add it in
    int mod_key = key | ((unsigned) INT_MAX + 1);
    bool succ = lf_add (&tab->buckets[ind], mod_key, pkt, &num_steps);
    if (!succ)
        return false;

    // See if we need to resize
    if (num_steps >= RESIZE_THRESH) {
        if (cap == PREALLOC_COUNT) {
            printf("Oops -- prealloc reached\n");
            exit(0);
        }
        __sync_bool_compare_and_swap(&tab->cap, cap, cap * 2);
    }

    return true;
}

bool split_remove(struct hashtable *_t, int key) {
    struct split_table *tab = (struct split_table *) _t;
    int cap = tab->cap;
    int ind = key & (cap - 1);
    // Make sure index is initialized
    if (!tab->buckets[ind].head)
        initialize_index(tab, ind);

    int mod_key = key | ((unsigned) INT_MAX + 1);
    return lf_remove (&tab->buckets[ind], mod_key);
}

bool split_contains(struct hashtable *_t, int key) {
    struct split_table *tab = (struct split_table *) _t;
    int cap = tab->cap;
    int ind = key & (cap - 1);
    // Make sure index is initialized
    if (!tab->buckets[ind].head)
        initialize_index(tab, ind);

    int mod_key = key | ((unsigned) INT_MAX + 1);
    int num_steps;
    bool succ = lf_contains (&tab->buckets[ind], mod_key, &num_steps);

    if (num_steps >= RESIZE_THRESH) {
        if (cap == PREALLOC_COUNT) {
            printf("Oops -- prealloc reached\n");
            exit(0);
        }
        __sync_bool_compare_and_swap(&tab->cap, cap, cap * 2);
    }

    return succ;
}

void spl_dump_table(struct split_table *tab) {
    printf("Table cap: %d\n", tab->cap);
    unsigned imax = INT_MAX;

    for (int i = 0; i < tab->cap; i++) {
        printf("%d: ", i);
        struct lf_elem *e = tab->buckets[i].head;
        while(e) {
            if ((e->key & (tab->cap - 1)) != i)
                break;
            if (e->key & (imax + 1)) {
                printf("%d ", e->key & imax);
            } else {
                printf("S%d ", e->key);
            }
            e = e->next;
        }
        printf("\n");
    }
}
