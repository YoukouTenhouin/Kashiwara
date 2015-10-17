CC = gcc
CFLAGS = -std=gnu99 -Wall -O3 -D_DEFAULT_SOURCE -I./include -pg
LDFLAGS = -pg -lp7
OBJS = mem/mempool.o http/request.o http/header.o http/response.o misc/util_heap.o

.PHONY: subdirs clean

kashiwara:$(OBJS) kashiwara.o
	$(CC) $(OBJS) kashiwara.o $(LDFLAGS) -okashiwara

%:
	$(CC) $(CFLAGS) -o$* $*.c -c 

clean:
	rm *.o */*.o
