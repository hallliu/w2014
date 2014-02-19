OBJS=locks.o queue.o packet_workers.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_locks.c test_workers.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

PERF_FILES=counter.c
PERF_OBJS=$(PERF_FILES:%.c=perf/%.so)

.PHONY: perfs tests clean

CC=gcc
override CFLAGS += -O3 -Wall -Werror -std=gnu99
#override CFLAGS += -O0 -Wall -Werror -std=gnu99 -g

LDLIBS=-pthread -lm

serial_firewall: serial_firewall.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(UTIL_OBJS) $< -o $@

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) -fPIC $< -o $@

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) -fPIC $< -o $@

tests/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

perf/counter.so: perf/counter.c $(OBJS) $(UTIL_OBJS)
	$(CC) -shared -fPIC $(LDFLAGS) $(CFLAGS) $(UTIL_OBJS) $(OBJS) $< -o $@ $(LDLIBS)

perfs: $(PERF_OBJS)

tests: $(TEST_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(TEST_OBJS) $(UTIL_OBJS) $(OBJS) -o tests/test_main $(LDLIBS)

clean:
	rm *.o Utils/*.o perf/*.so

testclean:
	rm tests/*.o tests/test_main
