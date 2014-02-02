OBJS=firewall.o queue.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_queue.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

.PHONY: tests

CC=gcc
CFLAGS=-O3 -Wall -Werror -std=gnu99
LDLIBS=-pthread -lm

firewall: $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(OBJS) $(UTIL_OBJS) -o firewall

firewall.o: firewall.c queue.o
queue.o: queue.c queue.h

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_queue.o: tests/test_queue.c queue.o
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_main.o: tests/test_main.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests: $(TEST_OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(TEST_OBJS) queue.o -o tests/test_main

clean:
	rm *.o Utils/*.o
