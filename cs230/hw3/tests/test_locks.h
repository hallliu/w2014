#ifndef TEST_LOCKS_H
#define TEST_LOCKS_H

void test_incrementing (char *, void *, int, int);
void test_hold_lock (char *, void *, int, int);
void test_lock_nohang (char *, void *, int);
void test_ordering (char *, void *, int);

#endif
