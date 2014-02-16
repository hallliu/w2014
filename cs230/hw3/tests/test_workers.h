#ifndef TEST_WORKERS_H
#define TEST_WORKERS_H

long general_test
        (int n_packets, 
         int n_src, 
         int worker_type,
         long mean, 
         int seed, 
         int distr,
         char *lock_type,
         void *lock_data);


void lastqueue_test(int n_packets, int n_src);
#endif
