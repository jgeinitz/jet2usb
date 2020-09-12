#
#
OBJS=commandline.o output.o commands.o datasocket.o

all: ../tool.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

../tool.a: $(OBJS)
	ar -r $@ $(OBJS)
	
clean:
	-/bin/rm -f *.o ../tool.a
