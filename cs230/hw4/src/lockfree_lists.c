#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <packetsource.h>

#include "lockfree_lists.h"

#define _UNUSED_ __attribute__((unused))
#define MARKOF(x) ((x) & (0x1))
#define REFOF(x) ((x) & (~0x1))
// Lookup table for bit-reversals copy-pasted from http://stackoverflow.com/a/746203
static char reverses[256] = {  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
      0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
          0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
            0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
              0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
                  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                      0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
                        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                          0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                            0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
                              0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                                0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
                                  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

static inline unsigned int rev_bits(int x) {
    return  ( (reverses[(x >> 24)])
            | (reverses[(x >> 16) & 0xff] << 8)
            | (reverses[(x >> 8) & 0xff] << 16)
            | (reverses[x & 0xff] << 24)
            );
}

struct lockfree_list *create_lockfree_list(void) {
    struct lockfree_list *l = malloc (sizeof(struct lockfree_list));
    l->head = NULL;
    l->size = 0;
    return l;
}

// Puts a pointer to the previous element in _prev.
// If the key is in the list, then _curr will contain the address of the element.
// If the key is not in the list, then _curr will contain the address of the element afterwards.
// If _prev is unset but _curr is, then that indicates that we found the key at the head or that it should go at the head.
// If _prev is set but _curr isn't, then that indicates that we didn't find the key and it should go at the tail
// If both are unset, that means that the list is currently empty.
void lf_find(volatile struct lockfree_list *l, unsigned rev_key, struct lf_elem **_prev, struct lf_elem **_curr) {
    struct lf_elem *prev = NULL, *curr = NULL, *next = NULL;
    while (1) {
restart_find:

        prev = l->head;
        if (prev == NULL)
            // This means list is empty. Do nothing to the two elements.
            return;

        curr = prev->next;

        // Do some logic to remove the marked elements in the beginning
        while (MARKOF(curr)) {
            bool successful_phy_rm = __sync_bool_compare_and_swap (&l->head, prev, REFOF(curr));
            if (!successful_phy_rm)
                goto restart_find;
            prev = REFOF(curr);
            curr = curr->next;
        }

        if (prev == NULL)
            return;

        if (rev_key <= prev->rev_key) {
            *_curr = REFOF(prev);
            return;
        }

        while (1) {
            if (curr == NULL) {
                *_prev = REFOF(prev);
                return;
            }
            next = curr->next;
            while (MARKOF(next)) {
                bool successful_phy_rm = __sync_bool_compare_and_swap (&prev->next, curr, next & (~0x1));
                if (!successful_phy_rm)
                    goto restart_find;
                curr = REFOF(next);
                next = curr->next;
            }

            if (rev_key < curr->rev_key) {
                *_prev = REFOF(prev)
                *_curr = REFOF(curr);
                return;
            }
            prev = REFOF(prev);
            curr = REFOF(curr);
        }
    }
}

bool lf_add(struct lockfree_list *l, int key, Packet_t *pkt) {
    unsigned rev_key = rev_bits(key);
    struct lf_elem *prev = NULL, *curr = NULL;
    // Allocate memory for this new elem now
    struct lf_elem *new_elem = malloc (sizeof(struct lf_elem));
    new_elem->pkt = pkt;
    new_elem->key = key;
    new_elem->rev_key = rev_key;

    while (1) {
        lf_find (l, rev_key, &prev, &curr);
        if (prev == NULL) {
            new_elem->next = curr;
            if (curr == NULL) {
                if (__sync_bool_compare_and_swap(&l->head, curr, new_elem))
                    return true;
                continue;
            }
            if (curr ->rev_key == rev_key) {
                free (new_elem);
                return false;
            }
            if (__sync_bool_compare_and_swap(&l->head, curr, new_elem))
                return true;
            continue;
        }

        if (curr && curr->rev_key == rev_key) {
            free (new_elem);
            return false;
        }

        new_elem->next = curr;
        if (__sync_bool_compare_and_swap(&prev->next, curr, new_elem)) {
            __sync_add_and_fetch (&l->size, 1);
            return true;
        }
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
            bool successful_phy_rm = __sync_bool_compare_and_swap (&curr->next, next, next | 0x1);
            if (!successful_phy_rm)
                continue;
            // Try a physical removal; no consequence if it fails.
            if (prev)
                __sync_bool_compare_and_swap (&prev->next, curr, REFOF(next));
            else
                __sync_bool_compare_and_swap (&l->head, curr, REFOF(next));
            return true;
        }
        return false;
    }
}

bool lf_contains (struct lockfree_list *l, int key) {
    unsigned key = rev_bits(key);
    struct lf_elem *curr = l->head;
    if (!curr)
        return false;
    int marked = 0;
    while (curr && rev_key > curr->rev_key) {
        marked = MARKOF(curr->next);
        curr = REFOF(curr->next);
    }
    return (curr && curr->rev_key == rev_key && !marked);
}

void destroy_lockfree_list (struct lockfree_list *l) {
    struct lf_elem *curr = l->head, *next = NULL;
    while (curr) {
        next = REFOF(curr->next);
        free (REFOF(curr));
        curr = next;
    }
    free (l);
}
