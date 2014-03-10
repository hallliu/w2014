#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "packet.h"
#include <hashpackettest.h>

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

    if (!strcmp(argv[1], "serial")) {
        serialHashPacketTest(atoi(argv[2]), atof(argv[3]), atof(argv[4]), 
                atof(argv[5]), atoi(argv[6]), atol(argv[7]), atoi(argv[8]));
        return 0;
    }

    if (!strcmp(argv[1], "parallel_noload")) {
        parallel_dispatcher(atoi(argv[2]), atoi(argv[3]), atof(argv[4]), 
                atof(argv[5]), atof(argv[6]), atoi(argv[7]), 0, atoi(argv[8]), "locked");
        return 0;
    }

    if (!strcmp(argv[1], "parallel")) {
        parallel_dispatcher(atoi(argv[2]), atoi(argv[3]), atof(argv[4]), 
                atof(argv[5]), atof(argv[6]), atoi(argv[7]), atol(argv[8]), atoi(argv[9]), argv[10]);
        return 0;
    }

    printf("Invalid perf name\n");
    return 1;
}
