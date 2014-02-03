OBJS=queue.o parallel_firewall.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_queue.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

.PHONY: tests

CC=gcc
override CFLAGS += -O0 -Wall -Werror -std=gnu99 -g

LDLIBS=-pthread -lm

firewall: $(OBJS) $(UTIL_OBJS) firewall.c
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(OBJS) $(UTIL_OBJS) firewall.c -o firewall

queue.o: queue.c queue.h
parallel_firewall.o: parallel_firewall.c parallel_firewall.h	

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_queue.o: tests/test_queue.c queue.o
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_main.o: tests/test_main.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests: $(TEST_OBJS) $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(TEST_OBJS) $(UTIL_OBJS) $(OBJS) -o tests/test_main

clean:
	rm *.o Utils/*.o

testclean:
	rm tests/*.o
