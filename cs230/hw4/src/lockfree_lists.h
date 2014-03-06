#ifndef LISTS_H
#define LISTS_H
#include <stdbool.h>

#include <packetsource.h>

struct lockfree_list {
    int FIXME;
};

struct lockfree_list *create_lockfree_list(void);
int lf_add (struct lockfree_list *, int, Packet_t *);
bool lf_remove (struct lockfree_list *, int);
bool lf_contains (struct lockfree_list *, int);
void destroy_lockfree_list (struct lockfree_list *);

#endif
