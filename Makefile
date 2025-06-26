CC = gcc
CFLAGS = -Wall -O2

all: p1 p2

p1: p1-dataProgram.c
	$(CC) $(CFLAGS) -o p1 p1-dataProgram.c

p2: p2-dataProgram.c
	$(CC) $(CFLAGS) -o p2 p2-dataProgram.c

run-server: p2
	./p2 &
	sleep 1

run1: p1
	./p1 1

run2: p1
	./p1 2

run: run-server run1

clean:
	rm -f p1 p2
