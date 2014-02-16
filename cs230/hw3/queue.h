#ifndef QUEUE_H
#define QUEUE_H
#include "locks.h"

struct l_queue {
    volatile int head, tail;
    int length;
    void **items;
    int n_enqueues;
    struct lock_t *lock;
    int queue_id;
};

struct l_queue *create_queues(int, int, char *, void *);
void destroy_queues(int, struct l_queue *);
int enq(struct l_queue *, void *);
int deq(struct l_queue *, void **);
int check_free(struct l_queue *);
int get_n_enqueues(struct l_queue *);

#endif
