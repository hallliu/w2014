#ifndef LOCKS_H
#define LOCKS_H
struct lock_t;

struct lock_t {
    void (*lock) (struct lock_t *);
    void (*unlock) (struct lock_t *);
    bool (*try_lock) (struct lock_t *);
    void (*destroy_lock) (struct lock_t *);
};
/*
struct lock_t *create_TAS(void);
struct lock_t *create_backoff(double min_delay, double max_delay);
struct lock_t *create_mutex(void);
struct lock_t *create_Alock(int max_threads);
struct lock_t *create_CLH(void);
struct lock_t *creat_MCS(void);
*/
struct lock_t *create_lock(char *, void *);
#endif
