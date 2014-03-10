#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../Utils/stopwatch.h"
#include "../Utils/hashgenerator.h"
#include "../src/queue.h"
#include "../src/hashtables.h"

#define QUEUE_DEPTH 16

static inline int blog2(int x) {
    int result = 0;
    while (x >>= 1, x)
        result++;
    return result;
}

static void usleep(int n) {
    struct timespec sleep_time = {.tv_sec = n / 1000000, .tv_nsec = (n % 1000000) * 1000l};
    nanosleep(&sleep_time, NULL);
}

void *alarm(void *_num_ms) {
    int *num_ms = (int *) _num_ms;
    usleep(*num_ms * 1000);
    *num_ms = 0;
    return NULL;
}

struct worker_data {
    struct hashtable *tab;
    struct l_queue *q;
    long fp_sum;
};

void *worker_fn(void *_data) {
    struct worker_data *data = (struct worker_data *) _data;
    struct l_queue *q = data->q;
    struct hashtable *tab = data->tab;
    HashPacket_t *pkt;
    while (1) {
        if (deq(q, (void **) &pkt))
            continue;
        if (pkt == NULL)
            break;
        data->fp_sum += getFingerPrint(pkt->body->iterations, pkt->body->seed);

        switch (pkt->type) {
            case Add:
                tab->add (tab, mangleKey(pkt), pkt->body);
                break;
            case Remove:
                tab->remove (tab, mangleKey(pkt));
                break;
            case Contains:
                tab->contains (tab, mangleKey(pkt));
                break;
        }
    }
    return NULL;
}

void *dropper_fn(void *_data) {
    struct worker_data *data = (struct worker_data *) _data;
    struct l_queue *q = data->q;
    while (1) {
        deq(q, (void **) &pkt)
        if (pkt == NULL)
            break;
    }
}

void parallel_dispatcher 
        (int n_ms,
         int n_src, 
         double add_frac,
         double rm_frac,
         double hit_rate,
         int preload_size,
         long mean, 
         int seed, 
         char *tab_type) {

    // set up the sources and receiver stuff
    bool drop_pkts = false;
    if (mean == 0) {
        mean = 1;
        drop_pkts = true;
    }
    
    HashPacketGenerator_t *pkt_src = createHashPacketGenerator(add_frac, rm_frac, hit_rate, mean);
    struct hashtable *tab = create_ht (tab_type, 1 << (blog2(n_src) + 1));


    // Preinsert elements
    for (int i = 0; i < preload_size; i++) {
        HashPacket_t *pkt = getAddPacket(pkt_src);
        tab->add (tab, mangleKey(pkt), pkt->body);
    }

    struct l_queue *queues = create_queues (n_src, QUEUE_DEPTH);
    struct worker_data *data = calloc (n_src, sizeof(struct worker_data));
    for (int i = 0; i < n_src; i++) {
        data[i].tab = tab;
        data[i].q = queues + i;
    }

    pthread_t *workers = calloc (n_src, sizeof(pthread_t));

    for (int i = 0; i < n_src; i++) {
        if (drop_pkts)
            pthread_create (workers + i, NULL, dropper_fn, (void *) (data + i));
        else
            pthread_create (workers + i, NULL, worker_fn, (void *) (data + i));
    }
    
    long packet_ctr = 0;

    // We pass the number of ms in to the alarm thread, and
    // the alarm thread sets it to 0 when time is up.
    volatile int not_done = n_ms;
    pthread_t alarm_thread;
    pthread_create(&alarm_thread, NULL, alarm, (void *) &not_done);

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
