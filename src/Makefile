
# all
CFLAGS=-g
LDFLAGS=-g

all: clean jet2usb

jet2usb: jet2usb.o tool.a
	gcc -o $@ jet2usb.o tool.a

jet2usb.o: jet2usb.c version.h
	gcc -c jet2usb.c

version.h:
	sh buildversion.sh version.h

tool.a:
	$(MAKE) -C functions
	cp functions/tool.a $@
	
clean:
	@/bin/rm -f jet2usb *.o version.h
	$(MAKE) -C functions clean

