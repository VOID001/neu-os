include ../Makefile.header

OBJS = getline.o printf.o

LDFLAGS	+= -r
CFLAGS += -I../include

.PHONY=clean run all

all: lib.o

lib.o: $(OBJS)
	$(LD) $(LDFLAGS) -o lib.o $(OBJS)

.c.o: 
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

clean:
	- rm -f $(OBJS) lib.o
