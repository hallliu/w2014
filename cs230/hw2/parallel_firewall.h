#ifndef P_FIREWALL_H
#define P_FIREWALL_H
#include <stdbool.h>

struct thread_data {
    struct l_queue *q;
#ifdef TESTING
    bool do_work;
#endif
};

#ifdef PERF
double parallel_dispatcher
#else
long parallel_dispatcher
#endif
        (int, 
        int, 
        int, 
        long, 
        int, 
#ifdef TESTING
        int,
#endif
        int);

void *worker_fn(void *);
#endif
