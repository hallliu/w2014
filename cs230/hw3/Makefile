OBJS=locks.o queue.o packet_workers.o
UTIL_FILES=generators.c crc32.c stopwatch.c fingerprint.c packetsource.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=test_main.c test_locks.c test_workers.c
TEST_OBJS=$(TEST_FILES:%.c=tests/%.o)

.PHONY: tests clean

CC=gcc
override CFLAGS += -O0 -Wall -Werror -std=gnu99 -g

LDLIBS=-pthread -lm

#firewall: $(OBJS) $(UTIL_OBJS) firewall.c
#	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $(OBJS) $(UTIL_OBJS) firewall.c -o firewall
all: serial_firewall

serial_firewall: serial_firewall.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

locks.o: locks.c locks.h

queue.o: queue.c queue.h

packet_workers.o: packet_workers.c packet_workers.h

Utils/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

#tests/test_locks.o: tests/test_locks.c
#	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@
#
#tests/test_workers.o: tests/test_workers.c
#	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@
#
#tests/test_main.o: tests/test_main.c
#	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

tests/%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@
	
tests: $(TEST_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(TEST_OBJS) $(UTIL_OBJS) $(OBJS) -o tests/test_main $(LDLIBS)

clean:
	rm *.o Utils/*.o

testclean:
	rm tests/*.o tests/test_main
