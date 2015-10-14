CC = gcc
CFLAGS = -std=gnu99 -Wall -O3 -D_DEFAULT_SOURCE -I./include -pg
LDFLAGS = -pg
OBJS = mem/mempool.o http/request.o http/header.o http/response.o event/evloop.o misc/util_heap.o

.PHONY: subdirs clean

izumo:$(OBJS) izumo.o
	$(CC) $(OBJS) $(LDFLAGS) -oizumo izumo.o

%:
	$(CC) $(CFLAGS) -o$* $*.c -c 

clean:
	rm *.o */*.o
