#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "fw.h"
#include "utils.h"

/* Assumptions on arguments: -n must be present, the number of nodes.
 * -t may or may not be present. If it is, then it's # threads. If not,
 *  indicates serial execution. If filename not present, read from stdin.
 */
int main(int argc, char *argv[]) {
    int n_nodes = 0, n_threads = 0;
    char *filename, *outfile = NULL;
    int opt;
    while (opt = getopt(argc, argv, "n:t:o:"), opt != -1) {
        switch (opt) {
            case 'n':
                n_nodes = (int) strtol(argv[optind], NULL, 10);
                break;
            case 't':
                n_threads = (int) strtol(argv[optind], NULL, 10);
                break;
            case 'o':
                outfile = argv[optind];
                break;
        }
    }
    if (optind >= argc) {
        filename = NULL;
    }
    else {
        filename = argv[optind];
    }
    /* Initialize adj matrix and fill it in */
    int *adj_matrix = malloc (n_nodes * n_nodes * sizeof(int));
    int *result;
    from_csv(adj_matrix, filename, n_nodes);
    if (n_threads == 0) {
        result = fw_serial(adj_matrix, n_nodes);
    } else {
        result = fw_parallel(adj_matrix, n_nodes, n_threads);
    }
    to_csv(result, outfile, n_nodes);
    return 0;
}

struct thread_info {
    // begin and end are inclusive.
    int begin;
    int end;
    int *adj;
    int n_nodes;
    pthread_barrier_t *bar;
};

void *fw_thread_worker(void *);
int *fw_parallel(int *adj, int n_nodes, int n_threads) {
    // Check if n_threads > n_nodes and restrict accordingly
    int nsq = n_nodes * n_nodes;
    if (n_threads > nsq) {
        n_threads = nsq;
    }
    // Initialize barrier
    pthread_barrier_t bar;
    pthread_barrier_init (&bar, NULL, n_threads);
    // Allocate block of thread infos and initialize
    struct thread_info *infos = malloc (n_threads * sizeof(struct thread_info));
    pthread_t *threads = malloc (n_threads * sizeof(pthread_t));
    for (int i = 0; i < n_threads; i++) {
        // See design doc for rationale behind this allocation.
        infos[i].begin = i * nsq / n_threads;
        infos[i].end= (i + 1) * nsq / n_threads - 1;
        infos[i].n_nodes = n_nodes;
        infos[i].bar = &bar;
        infos[i].adj = adj;
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, fw_thread_worker, (void *) (infos + i));
    }

    for(int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free (infos);
    free (threads);

    return adj;
}

void *fw_thread_worker(void *_info) {
    struct thread_info *info = (struct thread_info *) _info;
    int *adj = info->adj;

    for (int k = 0; k < info->n_nodes; k++) {
        int row_offset = info->begin / info->n_nodes * info->n_nodes;
        int col = info->begin - row_offset;
        int k_offset = k * info->n_nodes;

        for(int i = info->begin; i <= info->end; i++) {
            int alt_dist = adj[row_offset + k] + adj[k_offset + col];
            if (adj[i] > alt_dist)
                adj[i] = alt_dist;

            ++col;
            // TODO: This could have performance implications...
            if (__builtin_expect(!(col - info->n_nodes), 0)) {
                col = 0;
                row_offset += info->n_nodes;
            }
        }
        pthread_barrier_wait(info->bar);
    }
    return NULL;
}


/* Takes a adjacency matrix and modifies it in-place to produce a matrix
 * of shortest paths. Does not execute in parallel.
 */
int *fw_serial(int *adj, int n_nodes) {
    for (int k = 0; k < n_nodes; k++) {
        int k_start = k * n_nodes;
        for (int i = 0; i < n_nodes; i++) {
            int row_start = i * n_nodes;
            for(int j = 0; j < n_nodes; j++) {
                int alt_dist = adj[row_start + k] + adj[k_start + j];
                if (adj[row_start + j] > alt_dist)
                    adj[row_start + j] = alt_dist;
            }
        }
    }
    return adj;
}


