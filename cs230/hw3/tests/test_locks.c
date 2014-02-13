#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include "../locks.h"

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000};
    nanosleep(&sleep_time, NULL);
}


//------------------------------TEST 1----------------------------------//
// This test has the main thread hold a lock, then spawn workers that try
// to acquire it. None should succeed.

struct test_info1 {
    struct lock_t *l;
    volatile int *done;
};

static void *test_worker1(void *);

void test_hold_lock(char *lock_type, void *lock_info, int n_workers, int test_time) {
    struct lock_t *l = create_lock (lock_type, lock_info);
    l->lock(l);
    volatile int done = 0;
    struct test_info1 info = {.l = l, .done = &done};

    pthread_t *threads = calloc (n_workers, sizeof(pthread_t));
    for (int i = 0; i < n_workers; i++) {
        pthread_create (&threads[i], NULL, test_worker1, &info);
    }
    usleep(test_time);
    done = 1;
    for (int i = 0; i < n_workers; i++) {
        pthread_join (threads[i], NULL);
    }
    free (threads);
    l->destroy_lock(l);
    return;
}

static void *test_worker1(void *_info) {
    struct test_info1 *info = _info;
    struct lock_t *l = info->l;
    while (!info->done) {
        if (l->try_lock(l)) {
            printf("FAIL\n");
            l->unlock(l);
            break;
        }
    }
    return NULL;
}

//------------------------------TEST 2----------------------------------//
// This test has the workers increment a counter and atomically increment the value
// of the ith element of an array, where i is the last value of the counter.
// We expect counter_val to be divisible by n_workers.

struct test_info2 {
    struct lock_t *l;
    int num_incs;
    volatile int *counter;
    volatile char *array;
};

static void *test_worker2(void *);

void test_incrementing(char *lock_type, void *lock_info, int n_workers, int counter_val) {
    struct lock_t *l = create_lock (lock_type, lock_info);
    volatile int counter = 0;
    volatile char *ctr_array = calloc (counter_val, sizeof(char));

    struct test_info2 info = {
        .l = l,
        .num_incs = counter_val,
        .counter = &counter,
        .array = ctr_array,
    };
    
    pthread_t *threads = calloc (n_workers, sizeof(pthread_t));
    for (int i = 0; i < n_workers; i++) {
        pthread_create (&threads[i], NULL, test_worker2, &info);
    }

    for (int i = 0; i < n_workers; i++) {
        pthread_join (threads[i], NULL);
    }

    if (counter != counter_val) {
        printf("FAIL: final result of counter is %d\n", counter);
    }

    for (int i = 0; i < counter_val; i++) {
        if (ctr_array[i] != 1) {
            printf("FAIL: got %d for counter at %d\n", ctr_array[i], i);
            break;
        }
    }

    free (threads);
    free ((void *) ctr_array);
    l->destroy_lock(l);
    return;
}

static void *test_worker2(void *_info) {
    struct test_info2 *info = _info;
    struct lock_t *l = info->l;
    int upto = info->num_incs;

    while (1) {
        l->lock(l);
        if (*info->counter >= upto) {
            l->unlock(l);
            break;
        }
        int tmp = *info->counter;
        (*info->counter)++;
        __sync_fetch_and_add (info->array + tmp, 1);
        l->unlock(l);
    }
    return NULL;
}


//------------------------------TEST 3----------------------------------//
// This test makes sure that the locks don't hang. Locks and unlocks a lock
// repeatedly until time is up. test_time is in us as usual.

struct test_info3 {
    volatile int *done;
    int test_time;
};

static void *test3_alarm(void *);

void test_lock_nohang(char *lock_type, void *lock_info, int test_time) {
    struct lock_t *l = create_lock (lock_type, lock_info);
    volatile int done = 0;
    struct test_info3 info = {.done = &done, .test_time = test_time};

    pthread_t alarm;
    pthread_create(&alarm, NULL, test3_alarm, &info);
    while (!done) {
        l->lock(l);
        l->unlock(l);
    }
    pthread_join (alarm, NULL);
    l->destroy_lock(l);
    return;
}

static void *test3_alarm(void *_info) {
    struct test_info3 *info = _info;
    usleep (info->test_time);
    *info->done = 1;
    return NULL;
}



//------------------------------TEST 4----------------------------------//
// This test makes sure that the queue locks will let the threads go in order.

struct test_info4 {
    struct lock_t *l;
    int order_id;
};

static void *test4_worker(void *);

void test_ordering(char *lock_type, void *lock_info, int n_workers) {
    struct lock_t *l = create_lock (lock_type, lock_info);

    struct test_info4 *infos = calloc (n_workers, sizeof(struct test_info4));
    pthread_t *threads = calloc (n_workers, sizeof(pthread_t));

    for (int i = 0; i < n_workers; i++) {
        infos[i].l = l;
        infos[i].order_id = i;
        pthread_create (threads + i, NULL, test4_worker, infos + i);
    }

    for (int i = 0; i < n_workers; i++) {
        pthread_join (threads[i], NULL);
    }
    
    free (infos);
    free (threads);
    l->destroy_lock(l);
    return;
}

static void *test4_worker(void *_info) {
    struct test_info4 *info = _info;
    struct lock_t *l = info->l;

    int id = info->order_id;
    usleep ((id + 3) * 10000);
    l->lock(l);
    usleep (100000);
    printf("%d\n", id);
    l->unlock(l);
    return NULL;
}
