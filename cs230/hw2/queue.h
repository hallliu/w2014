#ifndef QUEUE_H
#define QUEUE_H

struct l_queue;

struct l_queue *create_queues(int, int);
void destroy_queues(int, struct l_queue *);
int enq(struct l_queue *, void *);
int deq(struct l_queue *, void **);
int check_free(struct l_queue *);

#endif
