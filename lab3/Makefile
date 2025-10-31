CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -lrt -pthread
TARGET = parent_shm child_shm

all: $(TARGET)

parent_shm: parent_shm.c
	$(CC) $(CFLAGS) -o parent_shm parent_shm.c

child_shm: child_shm.c
	$(CC) $(CFLAGS) -o child_shm child_shm.c

clean:
	rm -f $(TARGET) *.o *.txt

install: all
	cp parent_shm child_shm /usr/local/bin/

.PHONY: all clean install
