#ifndef PARALLEL_WORKERS_H
#define PARALLEL_WORKERS_H

#include "queue.h"

struct thread_data {
    struct l_queue *queues;
    int n_queues;
    int home_queue;
#ifdef TESTING
    // This is an int array of the same length as the number of queues.
    // The worker increments the ith entry by 1 when it dequeues from
    // the ith queue.
    int *queue_hits;
#endif
};

void *homequeue_worker (void *);
void *random_worker (void *);
void *lastqueue_worker (void *);
void *statistical_worker(void *);
void *statistical_worker1(void *);
#endif
