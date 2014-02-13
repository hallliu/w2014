OBJS=locks.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_locks.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

.PHONY: tests clean

CC=gcc
override CFLAGS += -O3 -Wall -Werror -std=gnu99

LDLIBS=-pthread -lm

#firewall: $(OBJS) $(UTIL_OBJS) firewall.c
#	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(OBJS) $(UTIL_OBJS) firewall.c -o firewall

locks.o: locks.c locks.h

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_locks.o: tests/test_locks.c locks.o
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/test_main.o: tests/test_main.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests: $(TEST_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(TEST_OBJS) $(UTIL_OBJS) $(OBJS) -o tests/test_main $(LDLIBS)

clean:
	rm *.o Utils/*.o

testclean:
	rm tests/*.o
