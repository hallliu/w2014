#include <stdlib.h>
#include "queue.h"

struct l_queue {
    volatile int head, tail;
    int queue_size;
    void **items;
};

struct l_queue *create_queue(int size) {
    struct l_queue *q = malloc (sizeof(struct l_queue));
    q->items = calloc (size, sizeof(void *));
    q->queue_size = size;
    q->head = 0;
    q->tail = 0;
    return q;
}

void destroy_queue(struct l_queue *q) {
    free (q->items);
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

    *obj_ptr = items[head % len];
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
