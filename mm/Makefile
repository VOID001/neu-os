include ../Makefile.header

LDFLAGS	+= -r
CFLAGS += -I../include

.PHONY=clean run all

.c.o:
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<
.s.o:
	@$(AS) -o $*.o $<

.c.s:
	@$(CC) $(CFLAGS) \
		-S -o $*.s $<

OBJS  = memory.o mm_test.o page.o

all: mm.o

mm.o: $(OBJS)
	@$(LD) $(LDFLAGS) -o mm.o $(OBJS)

clean:
	@rm -f core *.o *.a tmp_make
	@for i in *.c;do rm -f `basename $$i .c`.s;done
