#
# 
#
.EXPORT_ALL_VARIABLES:

all: build jet2usb

build:
	$(MAKE) -C src

jet2usb: src/jet2usb
	@cp $< $@

clean:
	$(MAKE) -C src $@
	-/bin/rm jet2usb
