include ../Makefile.header

CFLAGS += -I../include
LDFLAGS += -r

all: main.o

.c.o:
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

clean:
	rm -rf *.o
