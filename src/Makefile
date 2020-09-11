
# all
CFLAGS=-g
LDFLAGS=-g

all: clean jet2usb

jet2usb: jet2usb.o tool.a
	gcc -o $@ jet2usb.o tool.a
	
tool.a:
	$(MAKE) -C functions
	cp functions/tool.a $@

clean:
	@/bin/rm -f jet2usb *.o
	$(MAKE) -C functions clean
	
