#
# 
#
.EXPORT_ALL_VARIABLES:

all: build

build: src/jet2usb.c
	$(MAKE) -C src

clean:
	$(MAKE) -C src $@
