OBJS=locks.o queue.o packet_workers.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_locks.c test_workers.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

PERF_FILES=counter.c perf_main.c packet.c
PERF_OBJS=$(PERF_FILES:%.c=perf/%.o)

.PHONY: perf tests clean

CC=gcc
#override CFLAGS += -O3 -Wall -Werror -std=gnu99 -DTUNED_BACKOFF
override CFLAGS += -O0 -Wall -Werror -std=gnu99 -g -DTUNED_BACKOFF

LDLIBS=-pthread -lm

serial_firewall: serial_firewall.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(UTIL_OBJS) $< -o $@

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

perf: $(PERF_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(PERF_OBJS) $(UTIL_OBJS) $(OBJS) -o perf/perf_main $(LDLIBS)

tests: $(TEST_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(TEST_OBJS) $(UTIL_OBJS) $(OBJS) -o tests/test_main $(LDLIBS)

clean:
	rm *.o Utils/*.o perf/*.o perf/perf_main

testclean:
	rm tests/*.o tests/test_main
