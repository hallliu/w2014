#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_locks.h"

/*
 * Invoke with the test name as the first arg (excluding test_)
 * and additional arguments as subsequent
 * args.
 */
struct backoff_info {
    long min_delay;
    long max_delay;
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Specify test name\n");
        return 1;
    }

    if (!strcmp(argv[1], "hold_lock")) {
        if (!strcmp(argv[2], "backoff")) {
            struct backoff_info info = {atoi(argv[3]), atoi(argv[4])};
            test_hold_lock(argv[2], &info, atoi(argv[5]), atoi(argv[6]));
        }
        else if (!strcmp(argv[2], "Alock")) {
            int n_workers = atoi(argv[3]);
            test_hold_lock(argv[2], &n_workers, atoi(argv[4]), atoi(argv[5]));
        }
        else {
            test_hold_lock(argv[2], NULL, atoi(argv[3]), atoi(argv[4]));
        }
        return 0;
    }

    if (!strcmp(argv[1], "incrementing")) {
        if (!strcmp(argv[2], "backoff")) {
            struct backoff_info info = {atoi(argv[3]), atoi(argv[4])};
            test_hold_lock(argv[2], &info, atoi(argv[5]), atoi(argv[6]));
        }
        else if (!strcmp(argv[2], "Alock")) {
            int n_workers = atoi(argv[3]);
            test_hold_lock(argv[2], &n_workers, atoi(argv[4]), atoi(argv[5]));
        }
        else {
            test_hold_lock(argv[2], NULL, atoi(argv[3]), atoi(argv[4]));
        }
        return 0;
    }

    if (!strcmp(argv[1], "lock_nohang")) {
        if (!strcmp(argv[2], "backoff")) {
            struct backoff_info info = {atoi(argv[3]), atoi(argv[4])};
            test_hold_lock(argv[2], &info, atoi(argv[5]));
        }
        else if (!strcmp(argv[2], "Alock")) {
            int n_workers = atoi(argv[3]);
            test_hold_lock(argv[2], &n_workers, atoi(argv[4]));
        }
        else {
            test_hold_lock(argv[2], NULL, atoi(argv[3]));
        }
        return 0;
    }

    if (!strcmp(argv[1], "ordering")) {
        if (!strcmp(argv[2], "backoff")) {
            struct backoff_info info = {atoi(argv[3]), atoi(argv[4])};
            test_hold_lock(argv[2], &info, atoi(argv[5]), atoi(argv[6]));
        }
        else if (!strcmp(argv[2], "Alock")) {
            int n_workers = atoi(argv[3]);
            test_hold_lock(argv[2], &n_workers, atoi(argv[4]));
        }
        else {
            test_hold_lock(argv[2], NULL, atoi(argv[3]));
        }
        return 0;
    }

    printf("Invalid test name\n");
    return 1;
}
