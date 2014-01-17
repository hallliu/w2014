#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "fw.h"

int main(int argc, char **argv) {
    printf("stub\n");
}

int *fw_parallel(int *adj, int n_nodes, int n_threads) {
    printf("stub\n");
    return NULL;
}

int *fw_serial(int *adj, int n_nodes) {
    printf("stub\n");
    return NULL;
}
