#ifndef QUEUE_H
#define QUEUE_H

struct l_queue;

struct l_queue *create_queue(int);
void destroy_queue(struct l_queue *);
int enq(struct l_queue *, void *);
int deq(struct l_queue *, void **);

#endif
