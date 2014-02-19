#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "../queue.h"
#include "../Utils/generators.h"
#include "../Utils/stopwatch.h"
#include "../Utils/fingerprint.h"
#include "../Utils/packetsource.h"
#include "../packet_workers.h"

#define QUEUE_DEPTH 8

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000};
    nanosleep(&sleep_time, NULL);
}

long general_test
        (int n_packets, 
         int n_src, 
         int worker_type,
         long mean, 
         int seed, 
         int distr,
         char *lock_type,
         void *lock_data) {

    // set up the sources and receiver stuff
    Packet_t **rcvd_packets = malloc (n_packets * n_src * sizeof(Packet_t *));
    PacketSource_t *source = createPacketSource (mean, n_src, seed);

    Packet_t *(*pkt_fn)(PacketSource_t *, int) = distr ? &getUniformPacket : &getExponentialPacket;

    struct l_queue *queues = create_queues (n_src, QUEUE_DEPTH, lock_type, lock_data);

    void *(*worker_fn)(void *);
    switch (worker_type) {
        case 0:
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
    
    int n_workers_done = 0;
    int packet_ctr = 0;
    long total_fp_sum = 0;

    while (n_workers_done < n_src) {
        for (int i = 0; i < n_src; i++) {
            int n_enqs = get_n_enqueues(queues + i);
            if (n_enqs == n_packets + 1)
                continue;
            if (!check_free(queues + i))
                continue;
            
            if (n_enqs == n_packets) {
                enq(queues + i, NULL);
            }
            else {
                Packet_t *pkt = pkt_fn (source, i);
                enq (queues + i, (void *) pkt);

                rcvd_packets[packet_ctr] = pkt;
                packet_ctr += 1;
            }
 
            if (get_n_enqueues(queues + i) == n_packets + 1)
                n_workers_done += 1;
        }
    }

    for (int i = 0; i < n_src; i++) {
        long this_fp = 0;
        pthread_join (workers[i], (void **) &this_fp);
        total_fp_sum += this_fp;
    }

    // Print out the worker queue data. This is supposed to go straight
    // into Python from a stdout pipe.
   
    if (worker_type == 1) {
        for (int i = 0; i < n_src; i++) {
            for (int j = 0; j < n_src; j++) {
                printf("%d ", worker_data[i].queue_hits[j]);
            }
            free (worker_data[i].queue_hits);
            printf("\n");
        }
    }

    free (worker_data);
    destroy_queues (n_src, queues);

    deletePacketSource (source);
    for (int i = 0; i < packet_ctr; i++)
        free (rcvd_packets[i]);
    free (rcvd_packets);

    return total_fp_sum;
}

// Fills up n_src queues, then has a single worker process them with lastqueue. We don't expect
// all of them to get processed, but we do expect the ones that do to go in strict order
// i.e. queue i finishes before queue j starts if queue i starts before queue j.
void lastqueue_test(int n_packets, int n_src) {
    Packet_t **rcvd_packets = malloc (n_packets * n_src * sizeof(Packet_t *));
    PacketSource_t *source = createPacketSource (100, n_src, 0);

    struct l_queue *queues = create_queues (n_src, n_packets, "noop", NULL);

    struct thread_data worker_data = {.queues = queues, .n_queues = n_src, .home_queue = 0, .queue_hits = NULL};
    
    int packet_ctr = 0;
    for (int i = 0; i < n_src; i++) {
        for (int j = 0; j < n_packets; j++) {
            Packet_t *pkt = getUniformPacket (source, i);
            enq (queues + i, (void *) pkt);
            rcvd_packets[packet_ctr] = pkt;
            packet_ctr += 1;
        }
    }
    // At this point, all queues should be full. Let the worker loose on them. 
    
    pthread_t worker;
    pthread_create (&worker, NULL, lastqueue_worker, &worker_data);

    // Insert a NULL into the first queue so that the worker quits at some point
    usleep(n_packets * n_src / 2); 
    while (enq(queues, NULL));

    pthread_join (worker, NULL);

    destroy_queues (n_src, queues);

    deletePacketSource (source);
    for (int i = 0; i < packet_ctr; i++)
        free (rcvd_packets[i]);
    free (rcvd_packets);
    return;
}
