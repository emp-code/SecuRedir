CC=gcc
CFLAGS=-g -O3 -march=native -pipe -Wall -Wextra

objects = main.o

SecuRedir: $(objects)
	$(CC) $(CFLAGS) -o SecuRedir $(objects)

main: main.c

.PHONY: clean
clean:
	-rm $(objects)
