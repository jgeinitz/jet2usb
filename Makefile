#
# 
#
.EXPORT_ALL_VARIABLES:

all: build

build:
	$(MAKE) -C src

clean:
	$(MAKE) -C src $@
