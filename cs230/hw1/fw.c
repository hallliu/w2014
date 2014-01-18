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
                n_nodes = (int) strtol(argv[optind]);
                break;
            case 't':
                n_threads = (int) strtol(argv[optind]);
                break;
            case 'o':
                outfile = strcpy(argv[optind])
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

int *fw_parallel(int *adj, int n_nodes, int n_threads) {
    printf("stub\n");
    return NULL;
}

int *fw_serial(int *adj, int n_nodes) {
    printf("stub\n");
    return NULL;
}

