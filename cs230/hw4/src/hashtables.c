#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashtables.h"
#include "lockedtable.h"
#include "lfctable.h"

struct hashtable *create_ht(char *type, int capacity) {
    if (!strcmp(type, "locked")) {
        return (struct hashtable *) create_lockedtable(capacity);
    }

    if (!strcmp(type, "lfc")) {
        return (struct hashtable *) create_lfctable(capacity);
    }

    return NULL;
}
