#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "queue.h"
#include "locks.h"
#include "packet_workers.h"
#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"

void *homequeue_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

    struct l_queue *q = data->queues + data->home_queue;
    struct lock_t *lock = q->lock;
    Packet_t *pkt = NULL;
    long fp_sum = 0;
    while (1) {
        lock->lock (lock);
        if (deq(q, (void **) &pkt)) {
            lock->unlock (lock);
            continue;
        }
        if (pkt == NULL) {
            lock->unlock (lock);
            break;
        }
        fp_sum += getFingerprint (pkt->iterations, pkt->seed);
        lock->unlock (lock);
    }
    return ((void *) fp_sum);
}

void *random_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

    Packet_t *pkt = NULL;
    long fp_sum = 0;
    int queue_num;
    while (1) {
        queue_num = rand() % data->n_queues;
        struct lock_t *lock = (data->queues + queue_num)->lock;

        lock->lock (lock);
        if (deq(data->queues + queue_num, (void **) &pkt)) {
            lock->unlock (lock);
            continue;
        }
        if (pkt == NULL) {
            lock->unlock (lock);
            break;
        }
        fp_sum += getFingerprint (pkt->iterations, pkt->seed);
#ifdef TESTING
        data->queue_hits[queue_num] += 1;
#endif
        lock->unlock (lock);
    }
    return ((void *) fp_sum);
}

void *lastqueue_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

    Packet_t *pkt = NULL;
    long fp_sum = 0;
    int queue_num;

    while (1) {
        queue_num = rand() % data->n_queues;
        struct l_queue *q = data->queues + queue_num;
        struct lock_t *lock = q->lock;

        if (lock->try_lock (lock)) {
            // The case when the queue is empty on first encounter
            if (deq(q, (void **) &pkt)) {
                lock->unlock (lock);
                continue;
            }
            
            // The case when the queue is dead
            if (pkt == NULL) {
                lock->unlock (lock);
                break;
            }
            fp_sum += getFingerprint (pkt->iterations, pkt->seed);
            lock->unlock (lock);

            // Now loop and try to empty out the queue.
            while (1) {
                lock->lock (lock);

                // This is when the queue is empty. In this case, move on.
                if (deq(q, (void **) &pkt)) {
                    lock->unlock (lock);
                    break;
                }

                // If the queue is dead, we want to break out of the outer loop too.
                if (pkt == NULL) {
                    lock->unlock (lock);
                    return ((void *) fp_sum);
                }

                fp_sum += getFingerprint (pkt->iterations, pkt->seed);
                lock->unlock (lock);
            }
        }
    }

    return ((void *) fp_sum);
}

