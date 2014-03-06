#include <stdio.h>
#include <stdlib.h>

#include "../src/lists.h"

int main(int argc, char *argv[]) {
    char *test_type = argv[1];
   // if (!strcmp(test_type, "serial_persist"))
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
    while (e) {
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
