OBJS=src/serial_lists.o src/lockfree_lists.o src/lockedtable.o src/lfctable.o src/probetable.o src/splittable.o src/hashtables.o src/queue.o

UTIL_FILES=hashgenerator.c packetsource.c generators.c crc32.c hashpackettest.c hashpacketworker.c hashtable.c seriallist.c fingerprint.c stopwatch.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_lists.c test_hashtables.c
TEST_EXES=$(TEST_FILES:%.c=tests/%)

PERF_FILES=perf_main.c packet.c
PERF_OBJS=$(PERF_FILES:%.c=perf/%.o)

.PHONY: perf tests clean

CC=gcc
override CFLAGS += -O3 -Wall -Werror -std=gnu99 -IUtils
#override CFLAGS += -O0 -g -Wall -Werror -std=gnu99 -IUtils

LDLIBS=-pthread -lm

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

$(TEST_EXES): %: %.c $(UTIL_OBJS) $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(UTIL_OBJS) $(OBJS) $< -o $@ $(LDLIBS)

tests: $(TEST_EXES)

perf: $(PERF_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(PERF_OBJS) $(UTIL_OBJS) $(OBJS) -o perf/perf_main $(LDLIBS)

clean:
	rm src/*.o Utils/*.o $(TEST_EXES)
