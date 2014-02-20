#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
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
        lock->unlock (lock);
        fp_sum += getFingerprint (pkt->iterations, pkt->seed);
    }
    return ((void *) fp_sum);
}

void *random_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

    Packet_t *pkt = NULL;
    long fp_sum = 0;
    int queue_num;
    unsigned s_val = time(NULL);

    while (1) {
        queue_num = rand_r(&s_val) % data->n_queues;
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
        lock->unlock (lock);
        fp_sum += getFingerprint (pkt->iterations, pkt->seed);
#ifdef TESTING
        data->queue_hits[queue_num] += 1;
#endif
    }
    return ((void *) fp_sum);
}

void *lastqueue_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;

    Packet_t *pkt = NULL;
    long fp_sum = 0;
    int queue_num;

    unsigned s_val = time(NULL);
    while (1) {
        queue_num = rand_r(&s_val) % data->n_queues;

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
            lock->unlock (lock);
            fp_sum += getFingerprint (pkt->iterations, pkt->seed);

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
                lock->unlock (lock);

                fp_sum += getFingerprint (pkt->iterations, pkt->seed);
            }
        }
    }

    return ((void *) fp_sum);
}

// Constructs a cdf from the given pdf
void construct_cdf (const double *pdf, double *cdf, int len) {
    double pdf_sum = 0;
    for (int i = 0; i < len; i++) {
        pdf_sum += pdf[i];
        cdf[i] = pdf_sum;
    }
    for (int i = 0; i < len; i++)
        cdf[i] /= pdf_sum;
}

int sample_from_distr (const double *cdf, int len, unsigned *rand_seed) {
    double v = (double) rand_r(rand_seed) / RAND_MAX;
    for (int i = 0; i < len - 1; i++) {
        if (v < cdf[i])
            return i;
    }
    return (len - 1);
}

void *statistical_worker (void *_data) {
    struct thread_data *data = (struct thread_data *) _data;
    int queue_num;
    int queue_runs = 0;
    Packet_t *pkt = NULL;
    long fp_sum = 0;
    
    // pdf and cdf for sampling distributions
    double *source_pdf = malloc (data->n_queues * sizeof(double));
    double *source_cdf = malloc (data->n_queues * sizeof(double));

    for (int i = 0; i < data->n_queues; i++) {
        source_pdf[i] = 1;
    }

    construct_cdf(source_pdf, source_cdf, data->n_queues);

    unsigned s_val = time(NULL);
    while (1) {
        // Re-construct the cdf every 64 runs
        if ((queue_runs & 0x3F) == 0 && queue_runs)
            construct_cdf (source_pdf, source_cdf, data->n_queues);

        queue_num = sample_from_distr (source_cdf, data->n_queues, &s_val);
        struct l_queue *q = data->queues + queue_num;
        struct lock_t *lock = q->lock;

#ifdef TESTING
        data->queue_hits[queue_num] += 1;
#endif
        queue_runs++;

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
            
            if (!queue_runs)
                for (int i = 0; i < data->n_queues; i++)
                    source_pdf[i] = (double) pkt->iterations;

            lock->unlock (lock);
            fp_sum += getFingerprint (pkt->iterations, pkt->seed);

            source_pdf[queue_num] = 0.85 * source_pdf[queue_num] + 0.15 * pkt->iterations;

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
                lock->unlock (lock);

                fp_sum += getFingerprint (pkt->iterations, pkt->seed);
                source_pdf[queue_num] = 0.85 * source_pdf[queue_num] + 0.15 * pkt->iterations;
            }
        }
    }

    return ((void *) fp_sum);
}
