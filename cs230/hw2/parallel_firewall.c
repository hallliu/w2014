#include <stdio.h>
#include <stdlib.h>
#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"

struct thread_data {
    struct l_queue *q;
#ifdef TESTING
    bool do_work;
#endif
}

/*
 * distr=1 corresponds to uniform, 0 corresponds to exponential.
 */
#ifdef PERF
double parallel_dispatcher
#else
long parallel_dispatcher
#endif
        (int n_packets, 
        int n_src, 
        int q_depth, 
        long mean, 
        int seed, 
#ifdef TESTING
        int n_lazy,
#endif
        int distr) {

    // This array holds packets so we can free them in the end. Shouldn't be too big...
    // also we're putting packet memory management outside the timing for all three.

    Packet_t **rcvd_packets = malloc (n_packets * n_src * sizeof(Packet_t *));

    StopWatch_t watch;
    startTimer(&watch);

    struct l_queue *queues = create_queues (n_src, q_depth);
    PacketSource_t *source = createPacketSource (mean, n_src, seed);

    int n_workers_done = 0;
    int packet_ctr = 0;
    long total_fp_sum = 0;

    pthread_t *workers = calloc (n_src, sizeof(pthread_t));
    struct thread_data *worker_data = calloc (n_src, sizeof(struct thread_data));
    for (int i = 0; i < n_src; i++) {
        worker_data[i].q = queues + i;

#ifdef TESTING
        if (i >= n_lazy)
            worker_data[i].do_work = true;
#endif

        pthread_create (workers + i, NULL, worker_fn, (void *) worker_data + i);
    }
    
    Packet_t *(*pkt_fn)(PacketSource_t *, int) = distr ? getUniformPacket : getExponentialPacket;

    while (n_workers_done < n_src) {
        for (int i = 0; i < n_src; i++) {
            if (get_n_enqueues(queues + i) == n_packets)
                continue;
            if (!check_free(queues + i))
                continue;
            
            Packet_t *pkt = pkt_fn (source, i);
            enq (queues + i, (void *) pkt);

            rcvd_packets[packet_ctr] = pkt;
            packet_ctr += 1;

            if (get_n_enqueues(queues + i) == n_packets)
                n_workers_done += 1;
        }
#ifdef TESTING
        if (n_workers_done == n_src - n_lazy)
            break;
#endif
    }

    for (int i = 0; i < n_src; i++) {
        long this_fp = 0;
        pthread_join (workers[i], (void **) &this_fp);
        total_fp_sum += this_fp;
#ifdef TESTING
        printf("%d\n", get_n_enqueues(queues + i));
#endif
    }


    free (worker_data);
    destroy_queues (n_src, queues);
    deletePacketSource (source);

    stopTimer(&watch);

    // Free all the packets...
    for (int i = 0; i < packet_ctr * n_src; i++)
        free (rcvd_packets[i]);

    return getElapsedTime(&watch);
}

void *worker_fn(void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

#ifdef TESTING
    if (!data->do_work)
        return NULL;
#endif

    struct l_queue *q = data->q;
    Packet_t *pkt = NULL;
    long fp_sum = 0;

    while(1) {
        if (deq(q, (void **) &pkt))
            continue;
        if (pkt == NULL)
            break;
        fp_sum += getFingerPrint(pkt->iterations, pkt->seed);
    }
    
    return ((void *) fp_sum);
}
