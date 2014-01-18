#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "fw.h"

/* Assumptions on arguments: -n must be present, the number of nodes.
 * -t may or may not be present. If it is, then it's # threads. If not,
 *  indicates serial execution. If filename not present, read from stdin.
 */
int main(int argc, char *argv[]) {
    int n_nodes = 0, n_threads = 0;
    int opt;
    while (opt = getopt(argc, argv, "n:t:"), opt != -1) {
        switch (opt) {
            case 'n':
                n_nodes = (int) strtol(argv[optind]);
                break;
            case 't':
                n_threads = (int) strtol(argv[optind]);
                break;
        }
    }
    char *filename;
    if (optind >= argc) {
        filename = NULL;
    }
    else {
        char *filename = argv[optind];
    }
    /* Initialize adj matrix and fill it in */
    int *adj_matrix = malloc (n_nodes * n_nodes * sizeof(int));
    from_csv(adj_matrix, filename, n_nodes);
}

int *fw_parallel(int *adj, int n_nodes, int n_threads) {
    printf("stub\n");
    return NULL;
}

int *fw_serial(int *adj, int n_nodes) {
    printf("stub\n");
    return NULL;
}


