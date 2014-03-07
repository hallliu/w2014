#ifndef SERIAL_LISTS_H
#define SERIAL_LISTS_H
#include <stdbool.h>

#include <packetsource.h>

struct serial_list {
    struct serial_list_elem *head;
    volatile int size;
};

struct serial_list_elem {
    Packet_t *pkt;
    unsigned int key;
    unsigned int rev_key;
    struct serial_list_elem *next;
};

struct serial_list *create_serial_lists(int);
bool s_add (struct serial_list *, int, Packet_t *);
bool s_remove (struct serial_list *, int);
bool s_contains (struct serial_list *, int);
void destroy_serial_lists (struct serial_list *, int);
#endif
