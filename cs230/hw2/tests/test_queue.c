#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "testutils.h"
#include "../queue.h"

#define TIMELIMIT 2
static void eternal_dqr(void *_q);
static void eternal_eqr(void *_q);

static void eqr0(struct l_queue *, int n);
static void eqr1(struct l_queue *, int n);
static void eqr2(struct l_queue *, int n);
static void eqr3(struct l_queue *, int n);

static void dqr0(struct l_queue *, int n);
static void dqr1(struct l_queue *, int n);
static void dqr2(struct l_queue *, int n);
static void dqr3(struct l_queue *, int n);

static long timediff(struct timespec *start, struct timespec *end);
static void usleep(int us);
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
    while (gets(line_in), strcmp(line_in, "x")) {
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
    destroy_queue(q);
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
    pthread_create(&dqr, NULL, eternal_dqr, (void *) q);

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
    destroy_queue(q);
    return;
}

/*
 * Spawns a enqueuer thread that constantly tries to enqueue,
 * and the main thread dequeues n_to_enqueue times (successfully, hopefully)
 * Enqueuer thread will exit after two seconds.
 *
 * delay_mode controls the interval between successive dequeues.
 * 0 means dequeue as fast as possible
 * 1 means random small delays
 * 2 means large delay (0.5s) in the beginning, then fast dequeue
 * 3 means large delay, then random small delays.
 */
void test_queue_parallel_2 (int q_size, int n_to_dequeue, int delay_mode) {
    struct l_queue *q = create_queue(q_size);

    pthread_t dqr;
    pthread_create(&dqr, NULL, eternal_eqr, (void *) q);

    switch(delay_mode) {
        case 0:
            dqr0(q, n_to_enqueue);
            break;
        case 1:
            dqr1(q, n_to_enqueue);
            break;
        case 2:
            dqr2(q, n_to_enqueue);
            break;
        case 3:
            dqr3(q, n_to_enqueue);
            break;
    }

    pthread_join(&dqr, NULL);
    destroy_queue(q);
    return;
}

/* 
 * Function tries to dequeue from the queue in an endless loop, 
 * stopping after two seconds (checking once every hundred iterations)
 * Prints "FAIL" and exits if it receives a non-sequential value
 */
static void eternal_dqr(void *_q) {
    struct l_queue *q = (struct l_queue *) _q;
    struct timespec start, end;
    
    clock_gettime (CLOCK_MONOTONIC, &start);
    clock_gettime (CLOCK_MONOTONIC, &end);
    long val = 1;
    long old_val = 0;
    for (int i = 0;; i++) {
        if (!deq (q, (void **) &val)) {
            if (val == 0)
                break;
            if (old_val + 1 != val) {
                printf("FAIL: expected %ld, got %ld\n", old_val + 1, val);
                break;
            }
            old_val = val;
        }
        if (i % 100 == 0) {
            clock_gettime (CLOCK_MONOTONIC, &end);
            if (timediff (&start, &end) > 2000000) 
                break;
        }
    }
    return;
}

/* 
 * Function tries to enqueue from the queue in an endless loop, 
 * stopping after two seconds (checking once every hundred iterations)
 */
static void eternal_eqr(void *_q) {
    struct l_queue *q = (struct l_queue *) _q;
    struct timespec start, end;
    clock_gettime (CLOCK_MONOTONIC, &start);
    clock_gettime (CLOCK_MONOTONIC, &end);
    
    long val = 1;
    for (int i = 0;; i++) {
        if (!enq (q, (void *) val))
            val += 1;

        if (i % 100 == 0) {
            clock_gettime (CLOCK_MONOTONIC, &end);
            if (timediff (&start, &end) > 2000000) 
                break;
        }
    }
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
 * Following four functions perform the dequeuing of integers 1..n
 * in a manner perscribed by delay_mode (see above)
 */
static void dqr0(struct l_queue *q, int n) {
    long val = 1, old_val = 0;
    int success_ctr = 0;
    while (success_ctr < n) {
        if (!deq (q, (void **) &val)) {
            success_ctr++;
            if (old_val + 1 != val) {
                printf("FAIL: expected %ld, got %ld\n", old_val + 1, val);
                break;
            }
            old_val = val;
        }
    }
    return;
}

static void dqr1(struct l_queue *q, int n) {
    int total_sleep_us = 0;
    int sleep_limit = TIMELIMIT * 1000000 / 2;
    int max_per_sleep = sleep_limit / n;
    int success_ctr = 0;
    long val = 1, old_val = 0;
    
    while (success_ctr < n && total_sleep_us <= sleep_limit) {
        usleep(rand() % max_per_sleep + 1);
        if (!deq (q, (void **) val)) {
            success_ctr += 1;
            if (old_val + 1 != val) {
                printf("FAIL: expected %ld, got %ld\n", old_val + 1, val);
                break;
            }
            old_val = val;
        }
    }

    if (success_ctr != n) {
        while (success_ctr < n) {
            if (!deq (q, (void **) val)) {
                success_ctr += 1;
                if (old_val + 1 != val) {
                    printf("FAIL: expected %ld, got %ld\n", old_val + 1, val);
                    break;
                }
                old_val = val;
            }
        }
    }
    return;
}

static void dqr2(struct l_queue *q, int n) {
    usleep(500000);
    dqr0(q, n);
}

static void dqr3(struct l_queue *q, int n) {
    usleep(500000);
    dqr1(q, n);
}


/*
 * Sleeps for n microseconds.
 */
static void usleep(n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000};
    nanosleep(&sleep_time, NULL);
}

/* Returns time difference in microseconds */
static double timediff(struct timespec *start, struct timespec *end) {
    return ((end->tv_sec - start->tv_sec) * 1000000L + end->tv_usec - start->tv_usec) / 1000.0f;
}
