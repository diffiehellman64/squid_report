CC=gcc
CFLAGS=-Wall -ggdb -O2

OBJS= create_report.c
BINARY=squid_report

all: $(BINARY)

.c.o:
	$(CC) $(CFLAGS) -c $^ -o $@

$(BINARY): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *~ $(BINARY)

