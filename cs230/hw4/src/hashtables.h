#ifndef HASHTABLES_H
#define HASHTABLES_H
#include <stdbool.h>

#include <packetsource.h>
struct hashtable {
    bool (*add)(struct hashtable *, int, Packet_t *);
    bool (*remove)(struct hashtable *, int);
    bool (*contains)(struct hashtable *, int);
    volatile int cap;
};

struct hashtable *create_ht(char *type, int capacity);
#endif
