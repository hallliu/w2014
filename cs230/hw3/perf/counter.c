#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "../locks.h"
#include "../Utils/stopwatch.h"

#ifndef PERF
#error "This file should be compiled with -DPERF"
#endif

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000l};
    nanosleep(&sleep_time, NULL);
}

struct worker_data {
    volatile long counter;
    struct lock_t *lock;
    long work_amt;
    volatile int stop;
};

double serial_work_test(long val) {
    StopWatch_t w;
    startTimer (&w);
    volatile long ctr;
    for (long i = 0; i < val; i++) {
        ctr++;
    }

    stopTimer (&w);
    return getElapsedTime (&w);
}

void *work_worker (void *_data) {
    struct worker_data *data = (struct worker_data *) _data;
    long upto = data->work_amt;
    struct lock_t *l = data->lock;

    for (int i = 0; i < upto; i++) {
        l->lock (l);
        data->counter++;
        l->unlock (l);
    }
    return NULL;
}

double parallel_work_test(char *lock_type, void *lock_data, long val, int n_workers) {
    StopWatch_t w;

    struct lock_t *l;
    if (!strcmp(lock_type, "Alock"))
        l = create_lock (lock_type, &n_workers);
    else
        l = create_lock (lock_type, lock_data);

    
    struct worker_data data = {.counter = 0, .lock = l, .work_amt = val / n_workers};
    pthread_t *threads = malloc (n_workers * sizeof(pthread_t));

    startTimer (&w);
    for (int i = 0; i < n_workers; i++) {
        pthread_create (threads + i, NULL, work_worker, &data);
    }
    for (int i = 0; i < n_workers; i++) {
        pthread_join (threads[i], NULL);
    }
    stopTimer (&w);

    free (threads);
    l->destroy_lock (l);
    return getElapsedTime (&w);
}

void *time_worker (void *_data) {
    struct worker_data *data = (struct worker_data *) _data;

    struct lock_t *l = data->lock;

    while(!data->stop) {
        for (int i = 0; i < 100; i++) {
            l->lock (l);
            data->counter++;
            l->unlock (l);
        }
    }
    return NULL;
}

long serial_time_test (int n_ms) {
    // Use the noop lock to get rid of lock overhead in
    // the serial version
    struct lock_t *l = create_lock ("noop", NULL);
    pthread_t worker;
    struct worker_data data = {.lock = l, .counter = 0, .stop = 0};
    long final_value;

    pthread_create (&worker, NULL, time_worker, &data);
    usleep (n_ms * 1000);

    final_value = data.counter;
    data.stop = 1;
    pthread_join (worker, NULL);
    return final_value;
}

long parallel_time_test(char *lock_type, void *lock_data, int n_ms, int n_workers) {
    struct lock_t *l;
    if (!strcmp(lock_type, "Alock"))
        l = create_lock (lock_type, &n_workers);
    else
        l = create_lock (lock_type, lock_data);

    struct worker_data data = {.counter = 0, .lock = l, .stop = 0};
    pthread_t *threads = malloc (n_workers * sizeof(pthread_t));

    for (int i = 0; i < n_workers; i++) {
        pthread_create (threads + i, NULL, time_worker, &data);
    }

    usleep (n_ms * 1000);
    long final_value = data.counter;
    data.stop = 1;

    for (int i = 0; i < n_workers; i++) {
        pthread_join (threads[i], NULL);
    }

    free (threads);
    l->destroy_lock (l);
    return final_value;
}
