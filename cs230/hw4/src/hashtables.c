#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashtables.h"
#include "lockedtable.h"
#include "lfctable.h"
#include "splittable.h"
#include "probetable.h"

struct hashtable *create_ht(char *type, int capacity) {
    if (!strcmp(type, "locked")) {
        return (struct hashtable *) create_lockedtable(capacity);
    }

    if (!strcmp(type, "lfc")) {
        return (struct hashtable *) create_lfctable(capacity);
    }

    if (!strcmp(type, "probe")) {
        return (struct hashtable *) create_probetable(capacity * 4);
    }

    if (!strcmp(type, "split")) {
        return (struct hashtable *) create_splittable(capacity * 4);
    }

    return NULL;
}
