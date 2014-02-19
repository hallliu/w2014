#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "counter.h"

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

    if (!strcmp(argv[1], "serial_work")) {
        double time = serial_work_test(atol(argv[2]));
        printf("%.6f\n", time);
        return 0;
    }

    if (!strcmp(argv[1], "parallel_work")) {
        double time = parallel_work_test(argv[2], NULL, atol(argv[3]), atoi(argv[4]));
        printf("%.6f\n", time);
        return 0;
    }

    if (!strcmp(argv[1], "serial_time")) {
        long n = serial_time_test(atoi(argv[2]));
        printf("%ld\n", n);
        return 0;
    }

    if (!strcmp(argv[1], "parallel_time")) {
        long n = parallel_time_test(argv[2], NULL, atoi(argv[3]), atoi(argv[4]));
        printf("%ld\n", n);
        return 0;
    }

    printf("Invalid perf name\n");
    return 1;
}
