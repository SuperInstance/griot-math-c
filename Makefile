CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

all: libgriot.a test_griot

libgriot.a: src/griot.o
	ar rcs $@ $^

src/griot.o: src/griot.c src/griot.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_griot: tests/test_griot.c libgriot.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lgriot -lm

test: test_griot
	./test_griot

clean:
	rm -f src/*.o libgriot.a test_griot
