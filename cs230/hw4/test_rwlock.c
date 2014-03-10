#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct a {
    pthread_rwlock_t *l;
    volatile int *stop;
    int counter;
};

void *worker(void *_data) {
    struct a *data = (struct a *) _data;
    while (!(*data->stop)) {
        pthread_rwlock_rdlock(data->l);
        pthread_rwlock_unlock(data->l);
        data->counter++;
    }
    return NULL;
}
int main(int argc, char **argv) {
    int N = atoi(argv[1]);
    pthread_rwlock_t lock;
    pthread_rwlock_init(&lock, NULL);

    struct a *data = malloc(N*sizeof(struct a));
    int stop = 0;
    for (int i = 0; i < N; i++) {
        data[i].l = &lock;
        data[i].stop = &stop;
        data[i].counter = 0;
    }
    pthread_t *ts = malloc(N * sizeof(pthread_t));

    for (int i = 0; i < N; i++) {
        pthread_create(ts+i, NULL, worker, data + i);
    }

    usleep(2000000);
    stop = 1;

    for (int i = 0; i < N; i++) 
        pthread_join(ts[i], NULL);

    int ctr = 0;
    for (int i = 0; i < N; i++)
        ctr += data[i].counter;

    printf("%d\n", ctr);
}
