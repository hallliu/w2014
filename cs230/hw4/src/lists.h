#ifndef LISTS_H
#define LISTS_H
#include <stdbool.h>

#include <packetsource.h>

struct lockfree_list {
    int FIXME;
};

struct serial_list {
    volatile struct list_elem *head;
    volatile int size;
};

struct serial_list *create_serial_list(void);
int s_add (struct serial_list *, int, Packet_t *);
bool s_remove (struct serial_list *, int);
bool s_contains (struct serial_list *, int);
void destroy_serial_list (struct serial_list *);

struct lockfree_list *create_lockfree_list(void);
int lf_add (struct lockfree_list *, int, Packet_t *);
bool lf_remove (struct lockfree_list *, int);
bool lf_contains (struct lockfree_list *, int);
void destroy_lockfree_list (struct lockfree_list *);

#endif
