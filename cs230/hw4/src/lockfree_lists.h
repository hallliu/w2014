#ifndef LISTS_H
#define LISTS_H
#include <stdbool.h>

#include <packetsource.h>

struct lf_elem {
    Packet_t *pkt;
    unsigned key;
    unsigned rev_key;
    // We're going to use the LSB of this pointer as a mark.
    // It seems likely that malloc would align to word boundaries
    // which means that we'll get at least two zeros to play with.
    struct lf_elem *next;
};

struct lockfree_list {
    struct lf_elem *head;
    volatile int size;
};


struct lockfree_list *create_lockfree_list(void);
bool lf_add (struct lockfree_list *, int, Packet_t *);
bool lf_remove (struct lockfree_list *, int);
bool lf_contains (struct lockfree_list *, int);
void destroy_lockfree_list (struct lockfree_list *);

#endif
