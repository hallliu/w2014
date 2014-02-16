#include <stdlib.h>
#include "queue.h"
#include "locks.h"


struct l_queue *create_queues(int n_queues, int size, char *lock_type, void *lock_info) {
    struct l_queue *q = calloc (n_queues, sizeof(struct l_queue));
    for (int i = 0; i < n_queues; i++) {
        q[i].items = calloc (size, sizeof(void *));
        q[i].length = size;
        if (lock_type == NULL) 
            q[i].lock = create_lock("noop", NULL);
        else
            q[i].lock = create_lock(lock_type, lock_info);
    }
    return q;
}

void destroy_queues(int n_queues, struct l_queue *q) {
    for (int i = 0; i < n_queues; i++)
        free (q[i].items);
    free (q);
}

int enq(struct l_queue *q, void *obj) {
    int head = q->head;
    int tail = q->tail;
    int len = q->length;

    if (tail - head == len) {
        return 1;
    }

    q->items[tail % len] = obj;
    q->n_enqueues += 1;
    q->tail = tail + 1;
    return 0;
}

int deq(struct l_queue *q, void **obj_ptr) {
    q->lock->lock(q->lock);
    int head = q->head;
    int tail = q->tail;
    int len = q->length;

    if (tail == head) {
        q->lock->unlock(q->lock);
        return 1;
    }

    *obj_ptr = q->items[head % len];
    q->head = head + 1;
    q->lock->unlock(q->lock);
    return 0;
}

int check_free (struct l_queue *q) {
    int head = q->head;
    int tail = q->tail;
    int len = q->length;
  
    if (tail - head == len) {
        return 0;
    }
    else {
        return 1;
    }
}

int get_n_enqueues (struct l_queue *q) {
    return (q->n_enqueues);
}