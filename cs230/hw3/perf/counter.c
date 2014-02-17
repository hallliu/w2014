#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "locks.h"

#ifndef PERF
#error "This file should be compiled with -DPERF"
#endif

/* argv[1] specifies test type: 0 for work-based and 1 for time-based
 * argv[2] specifies work length: # of ms for time-based and counter val for work-based
 * argv[3] specifies parallelism: 1 for parallel, 0 for serial
 * argv[4] specifies number of threads, if parallelism is enabled
 * argv[5] specifies lock type, as a string
 */

int main (int argc, char *argv[]) {
    }
