CC=gcc
CFLAGS=-Wall -g

OBJECTS=tree_node.o tree.o main.o

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJECTS)
	$(CC) $(CFLAGS) -o main $(OBJECTS)


clean:
	rm -f ${OBJECTS}
	rm -f main