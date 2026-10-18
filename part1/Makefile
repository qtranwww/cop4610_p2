CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -O0

PROGRAMS := empty part1
TRACES := empty.trace part1.trace

.PHONY: all traces clean

all: $(PROGRAMS)

empty: empty.c
	$(CC) $(CFLAGS) -o $@ $<

part1: part1.c
	$(CC) $(CFLAGS) -o $@ $<

traces: $(PROGRAMS)
	strace -o empty.trace ./empty
	strace -o part1.trace ./part1

clean:
	rm -f $(PROGRAMS) $(TRACES)
