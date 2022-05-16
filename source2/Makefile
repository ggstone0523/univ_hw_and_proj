# Makefile

CC=gcc
CFLAGS=
OBJS=
LIBS=
all: sort mmap

sort: sort.c
	gcc -o sort sort.c

mmap: client_mmap.c sorter_mmap.c
	gcc -o client.mmap client_mmap.c
	gcc -o sorter.mmap sorter_mmap.c

clean:
	rm -f *.o sort client.mmap sorter.mmap
