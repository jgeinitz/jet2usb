

all: ../j2u

../j2u: main.o fun.o
	gcc -g main.o fun.o -o ../j2u

main.o: main.c version.h
	gcc -g -c main.c

fun.o: fun.c fun.h
	gcc -g -c fun.c

clean:
	@rm -f main.o fun.o ../j2u

build: all

