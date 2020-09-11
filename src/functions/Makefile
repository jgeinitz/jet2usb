#
#
CFLAGS=-g
OBJS=utilbox.o output.o commands.o datasocket.o

tool.a: $(OBJS)
	@ar -r tool.a $(OBJS)
	
clean:
	@/bin/rm *.o ../tool.a
	
