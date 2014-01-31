#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "testutils.h"
#include "../queue.h"

#define TIMELIMIT 2
static void usleep(int us);
static void eqr0(struct l_queue *, int n);
static void eqr1(struct l_queue *, int n);
static void eqr2(struct l_queue *, int n);
static void eqr3(struct l_queue *, int n);

/*
 * Tests serial correctness of the queue. Takes the queue capacity as a parameter
 * 
 * This function will wait for input on stdin. Input of the form "e (number)"
 * will enqueue the number, and input consisting of "d" will dequeue and print
 * to stdout. Function exits on "x".
 *
 * Unsuccessful enqueueing will output a line containing "F", unsuccessful dequeuing 
 * will output a line containing "E".
 */
void test_queue_serial(int q_size) {
    struct l_queue *q = create_queue(q_size);
    char *line_in = calloc(500, sizeof(char));

    long i = 0;
    while (fgets(&line_in, 500, stdin), strcmp(line_in, "x")) {
        if (line_in[0] == 'e') {
            i = strtol (line_in + 2, NULL, 10);
            if (enq (q, (void *) i)) {
                println("F");
            }
        }
        else {
            if (deq (q, (void **) &i)) {
                println("E");
            }
            else {
                printf("%d\n", i);
            }
        }
    }
    free (line_in);
    return;
}

/*
 * Spawns a dequeuer thread that constantly tries to dequeue,
 * and the main thread enqueues 1..n_to_enqueue values. Dequeuer thread
 * will exit after it dequeues a zero or after TIMELIMIT seconds,
 * whichever comes first.
 *
 * delay_mode controls the interval between successive enqueues.
 * 0 means enqueue as fast as possible
 * 1 means random small delays
 * 2 means large delay (0.5s) in the beginning, then fast enqueue
 * 3 means large delay, then random small delays.
 */
void test_queue_parallel_1 (int q_size, int n_to_enqueue, int delay_mode) {
    struct l_queue *q = create_queue(q_size);

    pthread_t dqr;
    pthread_create(&dqr, NULL, fast_dqr, (void *) q);

    switch(delay_mode) {
        case 0:
            eqr0(q, n_to_enqueue);
            break;
        case 1:
            eqr1(q, n_to_enqueue);
            break;
        case 2:
            eqr2(q, n_to_enqueue);
            break;
        case 3:
            eqr3(q, n_to_enqueue);
            break;
    }

    pthread_join(&dqr, NULL);
    return;
}

/*
 * Following four functions perform the enqueuing of integers 1..n
 * in a manner perscribed by delay_mode (see above)
 */
static void eqr0(struct l_queue *q, int n) {
    for (long i = 1; i <= n; i++) {
        if (enq (q, (void *) i)) {
            // If enq hits a full queue for some reason...
            i -= 1;
        }
    }
    while (enq (q, NULL)) {}; // Stick a zero into the queue.
    return;
}

static void eqr1(struct l_queue *q, int n) {
    int total_sleep_us = 0;
    int sleep_limit = TIMELIMIT * 1000000 / 2;
    int max_per_sleep = sleep_limit / n;
    long i = 1;
    
    while (i <= n && total_sleep_us <= sleep_limit) {
        usleep(rand() % max_per_sleep + 1);
        if (!enq (q, (void *) i))
            i += 1;
    }

    if (i != n + 1) {
        while (i <= n) {
            if (!enq (q, (void *) i))
                i += 1;
        }
    }

    while (enq (q, NULL)) {}; 
    return;
}

static void eqr2(struct l_queue *q, int n) {
    usleep(500000);
    eqr0(q, n);
}

static void eqr3(struct l_queue *q, int n) {
    usleep(500000);
    eqr1(q, n);
}

/*
 * Sleeps for n microseconds.
 */
static void usleep(n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000};
    nanosleep(&sleep_time, NULL);
}
