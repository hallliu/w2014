#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_queue.h"

/*
 * Invoke with the test name as the first arg (excluding test_)
 * and additional arguments as subsequent
 * args.
 */

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Specify test name\n");
        return 1;
    }

    if (!strcmp(argv[1], "queue_serial")) {
        test_queue_serial(atoi(argv[2]));
        return 0;
    }

    if (!strcmp(argv[1], "queue_parallel_1")) {
        test_queue_parallel_1(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        return 0;
    }

    if (!strcmp(argv[1], "queue_parallel_2")) {
        test_queue_parallel_2(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        return 0;
    }

    printf("Invalid test name\n");
    return 1;
}
