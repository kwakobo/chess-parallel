CC=gcc
CFLAGS=-Wall -g
LIB=-lpthread -lrt

OBJECTS=main.o tree_node.o tree.o

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LIB) -o main $(OBJECTS)


clean:
	rm -f ${OBJECTS}
	rm -f main