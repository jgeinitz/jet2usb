
#
.EXPORT_ALL_VARIABLES:

all: jet2usb

jet2usb: tool.a jet2usb.o
	$(CC) $(LDFLAGS) -o $@ jet2usb.o tool.a

jet2usb.o: version.h jet2usb.c
	$(CC) -c $(CFLAGS) jet2usb.c

version.h:
	sh tools/buildversion.sh version.h

tool.a:
	$(MAKE) -C functions
	cp functions/tool.a $@
	
clean:
	@/bin/rm -f jet2usb *.o version.h tool.a
	$(MAKE) -C functions clean

