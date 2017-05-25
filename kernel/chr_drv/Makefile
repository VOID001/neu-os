include ../../Makefile.header

OBJS = do_keyboard.o keyboard.o tty_io.o

LDFLAGS	+= -r
CFLAGS += -I../../include

.PHONY=clean run all
 
all: chr_drv.a

chr_drv.a: $(OBJS)
	@$(AR) rcs chr_drv.a $(OBJS)
	@sync

.c.o: 
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

clean:
	- rm -f $(OBJS) *.a
