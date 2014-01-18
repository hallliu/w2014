#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

/* Grabs an int matrix from the named csv file and loads it
 * into the int array. Assumes that the csv file contains a
 * well-formed matrix and has a terminating , or newline.
 */
void from_csv(int *matrix, char *filename, int n) {
    FILE *f;
    if (filename != NULL) {
        f = fopen(filename, "r");
    } else {
        f = stdin;
    }

    int matr_ctr = 0, num_ctr = 0;
    char *num_buf = malloc (9); // Longest number possible has 8 characters
    while (!feof(f)) {
        num_buf[num_ctr] = fgetc(f);
        if (num_buf[num_ctr] == ',' || num_buf[num_ctr] == '\n') {
            num_buf[num_ctr] = '\0';
            matrix[matr_ctr] = (int) strtol (num_buf, NULL, 10);
            num_ctr = 0;
            matr_ctr++;
        } else {
            num_ctr++;
        }
    }
    assert (matr_ctr == n * n);
    free (num_buf);
    if (f != stdin) {
        fclose (f);
    }
    return;
}

/* Takes the int matrix and prints it to a csv file. If filename is null, print to stdout
 */
void to_csv(int *matrix, char *filename, int n) {
    FILE *f;
    if (filename != NULL) {
        f = fopen(filename, "w");
    } else {
        f = stdout;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - 1; j++) {
            fprintf(f, "%d,", matrix[i * n + j]);
        }
        fprintf (f, "%d\n", matrix[(i - 1) * n + 1]);
    }
    if (f != stdout) {
        fclose (f);
    }
    return;
}
