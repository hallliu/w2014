#include <stdlib.h>
#include "queue.h"


struct l_queue *create_queues(int n_queues, int size) {
    struct l_queue *q = calloc (n_queues, sizeof(struct l_queue));
    for (int i = 0; i < n_queues; i++) {
        q[i].items = calloc (size, sizeof(void *));
        q[i].length = size;
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
    int head = q->head;
    int tail = q->tail;
    int len = q->length;

    if (tail == head) {
        return 1;
    }

    *obj_ptr = q->items[head % len];
    q->head = head + 1;
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
