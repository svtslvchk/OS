CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = parent child

all: $(TARGET)

parent: parent.c
	$(CC) $(CFLAGS) -o parent parent.c

child: child.c
	$(CC) $(CFLAGS) -o child child.c

clean:
	rm -f $(TARGET) *.txt

install: all
	cp parent child /usr/local/bin/

.PHONY: all clean install