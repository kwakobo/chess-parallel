CC=gcc
CFLAGS=-Wall -g
LIB=-lpthread -lrt

OBJECTS=tree_node.o tree.o

all: main main_nocnt

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJECTS) main.o
	$(CC) $(CFLAGS) $(LIB) -o main $(OBJECTS) main.o

main_nocnt: $(OBJECTS) main_nocnt.o
	$(CC) $(CFLAGS) $(LIB) -o main_nocnt $(OBJECTS) main_nocnt.o

.PHONY: clean
clean:
	rm -f ${OBJECTS}
	rm -f main