#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../locks.h"
#include "../Utils/stopwatch.h"
#include "../queue.h"
#include "../packet_workers.h"

#ifndef PERF
#error "This file should be compiled with -DPERF"
#endif

#ifndef TUNED_BACKOFF
#error "This file needs the tuned version of the backoff lock"
#endif

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000l};
    nanosleep(&sleep_time, NULL);
}

void *alarm(void *_num_ms) {
    int *num_ms = (int *) _num_ms;
    usleep(*num_ms);
    *num_ms = 0;
}

void parallel_dispatcher 
        (int n_ms,
         int n_src, 
         int worker_type,
         long mean, 
         int seed, 
         int distr,
         char *lock_type) {

    // set up the sources and receiver stuff
    PacketSource_t *source = createPacketSource (mean, n_src, seed);

    Packet_t *(*pkt_fn)(PacketSource_t *, int) = distr ? &getUniformPacket : &getExponentialPacket;

    struct l_queue *queues = create_queues (n_src, QUEUE_DEPTH, lock_type, &n_src);

    void *(*worker_fn)(void *);
    switch (worker_type) {
        case 0:
            // homequeue worker with the noop lock is equivalent to lockfree.
            worker_fn = homequeue_worker;
            break;
        case 1:
            worker_fn = random_worker;
            break;
        default:
            worker_fn = lastqueue_worker;
            break;
    }

    struct thread_data *worker_data = calloc (n_src, sizeof(struct thread_data));
    pthread_t *workers = calloc (n_src, sizeof(pthread_t));
    for (int i = 0; i < n_src; i++) {
        worker_data[i].queues = queues;
        worker_data[i].n_queues = n_src;
        worker_data[i].home_queue = i;
        worker_data[i].queue_hits = calloc (n_src, sizeof(int));

        pthread_create (workers + i, NULL, worker_fn, (void *) (worker_data + i));
    }
    
    long packet_ctr = 0;

    // We pass the number of ms in to the alarm thread, and
    // the alarm thread sets it to 0 when time is up.
    volatile int not_done = n_ms;
    pthread_t alarm_thread;
    pthread_create(alarm_thread, NULL, alarm, &not_done);

    while (not_done) {
        for (int i = 0; i < n_src; i++) {
            if (!check_free(queues + i))
                continue;
            Packet_t *pkt = pkt_fn (source, i);
            enq (queues + i, (void *) pkt);

            packet_ctr += 1;
        }
    }
    printf("%ld\n", packet_ctr);

    // At this point, screw cleanup. It doesn't matter for the results,
    // so let the OS take care of it for us.
    exit(0);
}
