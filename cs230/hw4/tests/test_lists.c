#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

#include "../src/serial_lists.h"
#include "../src/lockfree_lists.h"

int *key_helper(int N, int limit);

void serial_contains (int N);
void serial_removals(int N);
void serial_nocontains (int N);
void serial_ordering (int N);

int main(int argc, char *argv[]) {
    char *test_type = argv[1];
    if (!strcmp(test_type, "serial_contains")) {
        serial_contains(atoi(argv[2]));
        return 0;
    }

    if (!strcmp(test_type, "serial_removals")) {
        serial_removals(atoi(argv[2]));
        return 0;
    }

    if (!strcmp(test_type, "serial_nocontains")) {
        serial_nocontains(atoi(argv[2]));
        return 0;
    }

    if (!strcmp(test_type, "serial_ordering")) {
        serial_ordering(atoi(argv[2]));
        return 0;
    }

    printf("Invalid test\n");
    return 1;
}


// Inserts N distinct random elements into a list, then tests that they're still all there.
void serial_contains (int N) {
    int *keys = malloc (N * sizeof(int));
    srand (time(0));
    struct serial_list *l = create_serial_list();

    for (int i = 0; i < N; i++) {
        keys[i] = rand();
        while (!s_add (l, keys[i], NULL)) {
            keys[i] = rand();
        }
    }

    for (int i = 0; i < N; i++) {
        if (!s_contains(l, keys[i])) {
            printf("FAIL: missing\n");
            goto end;
        }
    }

end:
    destroy_serial_list (l);
    free(keys);
}

// Inserts N distinct random elements into the list, removes half of them, 
// then tests that contains functions correctly.
void serial_removals(int N) {
    int *keys = malloc (N * sizeof(int));
    srand (time(0));
    struct serial_list *l = create_serial_list();

    for (int i = 0; i < N; i++) {
        keys[i] = rand();
        while (!s_add (l, keys[i], NULL)) {
            keys[i] = rand();
        }
    }

    for (int i = 0; i < N / 2; i++) {
        if (!s_remove(l, keys[i])) {
            printf("FAIL: remove failure\n");
            goto end;
        }
    }

    for (int i = 0; i < N / 2; i++) {
        if (s_contains(l, keys[i])) {
            printf("FAIL: still contains\n");
            goto end;
        }
    }

    for (int i = N / 2; i < N; i++) {
        if (!s_contains(l, keys[i])) {
            printf("FAIL: missing\n");
            goto end;
        }
    }

end:
    destroy_serial_list (l);
    free(keys);

}

// Inserts N random elements with keys <=2N, then tests 20 random keys above 2N to make sure
// that they're not contained.
void serial_nocontains (int N) {
    int *keys = malloc (N * sizeof(int));
    srand (time(0));
    struct serial_list *l = create_serial_list();

    for (int i = 0; i < N; i++) {
        keys[i] = rand() % (N * 2);
        s_add (l, keys[i], NULL);
    }

    for (int i = 0; i < 20; i++) {
        int shouldnt_be_in = rand() + 2 * N;
        if (s_contains (l, shouldnt_be_in)) {
            printf("FAIL\n");
            break;
        }
    }
    destroy_serial_list (l);
    free(keys);
}

// Inserts a bunch of random elements in, and tests that the ordering makes sense.
void serial_ordering (int N) {
    if (N < 2)
        return; //no sense in ordering one element

    int *keys = malloc (N * sizeof(int));
    srand (time(0));
    struct serial_list *l = create_serial_list();

    for (int i = 0; i < N; i++) {
        keys[i] = rand();
        while (!s_add (l, keys[i], NULL)) {
            keys[i] = rand();
        }
    }
    
    struct serial_list_elem *e = l->head;
    while (e->next) {
        if (e->rev_key >= e->next->rev_key) {
            printf("FAIL: key misorder\n");
            goto end;
        }
        e = e->next;
    }

end:
    destroy_serial_list (l);
    free(keys);
}

// Parallel tests start here --------------------------------------

// Makes an array of N distinct random values with upper bound limit. Uses
// the (presumably correct) serial list to guarantee distinctness. 
int *key_helper(int N, int limit) {
    int *keys = malloc (N * sizeof(int));
    srand (time(0));
    struct serial_list *l = create_serial_list();

    for (int i = 0; i < N; i++) {
        keys[i] = rand() % limit;
        while (!s_add (l, keys[i], NULL)) {
            keys[i] = rand() % limit;
        }
    }
    destroy_serial_list (l);
    return keys;
}

// Parallel worker -- takes a struct with a array of keys, 
// a array of operations (0=contains, 1=add, 2=remove), a begin
// and an end (both inclusive), and performs
// the requested operations, then stores the results in the
// preallocated results array 
typedef struct parallel_worker_data {
    struct lockfree_list *l;
    int *keys;
    int *ops;
    int begin;
    int end;
    bool *results;
} pdata;

void *parallel_worker(void *_data) {
    pdata *data = (pdata) data;
    struct lockfree_list *l = data->l;
    int *keys = data->keys;
        
    for (int i = begin; i <= end; i++) {
        switch (data->ops[i]) {
            case 0:
                data->results[i] = lf_contains(l, keys[i]);
                break;
            case 1:
                data->results[i] = lf_add(l, keys[i], NULL);
                break;
            default:
                data->results[i] = lf_remove(l, keys[i]);
                break;
        }
    }
    return NULL;
}

// Adds in N distinct keys to the list using T threads, then uses T threads
// to test that they're in the list.
void parallel_addcontain(int N, int T) {
    int *keys = key_helper(N, INT_MAX);
    bool *results = malloc(N * sizeof(bool));
    int *ops = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) 
        ops[i] = 1;

    if (T > N)
        T = N;

    struct lockfree_list *l = create_lockfree_list();

    pdata *datas = malloc (T * sizeof(pdata));
    pthread_t *threads = malloc(T * sizeof(pthread_t));
    for (int i = 0; i < T; i++) {
        datas[i].l = l;
        datas[i].keys = keys;
        datas[i].ops = ops;
        datas[i].begin = i * N / T;
        datas[i].end = (i + 1) * N / T - 1;
        datas[i].results = results;
    }

    for (int i = 0; i < T; i++) 
        pthread_create(&threads[i], NULL, parallel_worker, (void *)(datas + i));

    for (int i = 0; i < T; i++) 
        pthread_join(threads[i], NULL);

    for (int i = 0; i < N; i++) {
        if (!results[i]) {
            printf("FAIL: bad return on parallel adds\n");
            return;
        }
        ops[i] = 0; //convert it over to a containment operation
    }

    for (int i = 0; i < T; i++) 
        pthread_create(&threads[i], NULL, parallel_worker, (void *)(datas + i));

    for (int i = 0; i < T; i++) 
        pthread_join(threads[i], NULL);

    for (int i = 0; i < N; i++) {
        if (!results[i]) {
            printf("FAIL: bad return on parallel contains\n");
            return;
        }
    }
    return;
}
