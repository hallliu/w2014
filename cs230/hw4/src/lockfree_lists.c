#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <packetsource.h>

#include "lockfree_lists.h"

#define _UNUSED_ __attribute__((unused))
#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (typeof(x))((unsigned long)(x) & (~0x1l))
#define MARKED(x) (typeof(x))((unsigned long)(x) | 0x1l)
struct lockfree_list *create_lockfree_lists(int n) {
    struct lockfree_list *l = malloc (n * sizeof(struct lockfree_list));
    for (int i = 0; i < n; i++) {
        l[i].head = NULL;
        l[i].size = 0;
    }
    return l;
}

// Puts a pointer to the previous element in _prev.
// If the key is in the list, then _curr will contain the address of the element.
// If the key is not in the list, then _curr will contain the address of the element afterwards.
// If _prev is unset but _curr is, then that indicates that we found the key at the head or that it should go at the head.
// If _prev is set but _curr isn't, then that indicates that we didn't find the key and it should go at the tail
// If both are unset, that means that the list is currently empty.
// Return value is the number of steps taken
int lf_find(volatile struct lockfree_list *l, unsigned rev_key, struct lf_elem **_prev, struct lf_elem **_curr) {
    volatile struct lf_elem *prev = NULL, *curr = NULL, *next = NULL;
    while (1) {
restart_find:

        prev = l->head;
        if (prev == NULL)
            // This means list is empty. Do nothing to the two elements.
            return 0;

        curr = prev->next;

        // Do some logic to remove the marked elements in the beginning
        while (MARKOF(curr)) {
            bool successful_phy_rm = __sync_bool_compare_and_swap (&l->head, prev, REFOF(curr));
            if (!successful_phy_rm)
                goto restart_find;
            prev = REFOF(curr);
            curr = prev->next;
        }

        if (prev == NULL)
            return 0;

        if (rev_key <= prev->rev_key) {
            *_curr = (struct lf_elem *) REFOF(prev);
            return 0;
        }
        int num_steps = 0;

        while (1) {
            if (curr == NULL) {
                *_prev = (struct lf_elem *) REFOF(prev);
                return num_steps;
            }
            next = curr->next;
            while (MARKOF(next)) {
                bool successful_phy_rm = __sync_bool_compare_and_swap (&prev->next, curr, REFOF(next));
                if (!successful_phy_rm)
                    goto restart_find;
                curr = REFOF(next);
                next = curr->next;
            }

            if (rev_key <= curr->rev_key) {
                *_prev = (struct lf_elem *) REFOF(prev);
                *_curr = (struct lf_elem *) REFOF(curr);
                return num_steps;
            }
            prev = REFOF(curr);
            curr = REFOF(curr->next);
            num_steps++;
        }
    }
}

bool lf_add(struct lockfree_list *l, int key, Packet_t *pkt, int *num_steps) {
    unsigned rev_key = rev_bits(key);
    struct lf_elem *prev = NULL, *curr = NULL;
    // Allocate memory for this new elem now
    struct lf_elem *new_elem = malloc (sizeof(struct lf_elem));
    new_elem->pkt = pkt;
    new_elem->key = key;
    new_elem->rev_key = rev_key;

    while (1) {
        if (num_steps)
            *num_steps = lf_find (l, rev_key, &prev, &curr);
        else
            lf_find (l, rev_key, &prev, &curr);

        __sync_add_and_fetch (&l->size, 1);
        if (prev == NULL) {
            new_elem->next = curr;
            if (curr == NULL) {
                if (__sync_bool_compare_and_swap(&l->head, curr, new_elem)) {
                    return true;
                }
                __sync_add_and_fetch (&l->size, -1);
                continue;
            }
            if (curr ->rev_key == rev_key) {
                free (new_elem);
                __sync_add_and_fetch (&l->size, -1);
                return false;
            }
            if (__sync_bool_compare_and_swap(&l->head, curr, new_elem)) {
                return true;
            }
            __sync_add_and_fetch (&l->size, -1);
            continue;
        }

        if (curr && curr->rev_key == rev_key) {
            __sync_add_and_fetch (&l->size, -1);
            free (new_elem);
            return false;
        }

        new_elem->next = curr;
        if (__sync_bool_compare_and_swap(&prev->next, curr, new_elem)) {
            return true;
        }
        __sync_add_and_fetch (&l->size, -1);
    }
}

bool lf_remove (struct lockfree_list *l, int key) {
    unsigned rev_key = rev_bits (key);
    struct lf_elem *prev = NULL, *curr = NULL, *next = NULL;
    while (1) {
        lf_find (l, rev_key, &prev, &curr);
        if (curr) {
            if (curr->rev_key != rev_key) {
                return false;
            }
            
            next = curr->next;
            bool successful_log_rm = __sync_bool_compare_and_swap (&curr->next, next, MARKED(next));
            if (!successful_log_rm)
                continue;
            // Try a physical removal; no consequence if it fails.
            if (prev)
                __sync_bool_compare_and_swap (&prev->next, curr, REFOF(next));
            else
                __sync_bool_compare_and_swap (&l->head, curr, REFOF(next));
            __sync_add_and_fetch (&l->size, -1);
            return true;
        }
        return false;
    }
}

bool lf_contains (struct lockfree_list *l, int key, int *num_steps) {
    unsigned rev_key = rev_bits(key);
    struct lf_elem *curr = l->head;
    int n_steps = 0;

    if (!curr) {
        return false;
    }

    while (curr && rev_key > curr->rev_key) {
        n_steps++;
        curr = REFOF(curr->next);
    }
    if (num_steps)
        *num_steps = n_steps;

    return (curr && curr->rev_key == rev_key && !MARKOF(curr->next));
}

void destroy_lockfree_lists (struct lockfree_list *l, int n) {
    for (int i = 0; i < n; i++) {
        struct lf_elem *curr = l[i].head, *next = NULL;
        while (curr) {
            next = REFOF(curr->next);
            free (REFOF(curr));
            curr = next;
        }
    }
    free (l);
}
