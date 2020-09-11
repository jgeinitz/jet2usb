#

#
OBJS=utilbox.o
SRCS=utilbox.c
INCS=utilbox.h

../tool.a: $(OBJS)
	ar -r ../tool.a $(OBJS)
	
utilbox.o: utilbox.c
	gcc -c utilbox.c
	
clean:
	@/bin/rm *.o ../tool.a
	