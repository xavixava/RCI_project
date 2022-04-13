#   Compiler, Compiler Flags

CC = gcc
CFLAGS = -g -Wall  

#   Sources
SOURCES = main.c 

#   Objects  ('make' automatically compiles .c to .o)
OBJECTS = main.o interface.o node.o network.o hash.o

ring: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

main.o: main.c interface.h 

interface.o: interface.c interface.h 

node.o: node.c node.h 

network.o: network.c network.h

hash.o: hash.c hash.h

clean:
	rm -f *.o  ring

FILES = $(shell ls ../test/*.txt)

VALG = valgrind --leak-check=full

valgrind: ring
	$(VALG) ./ring

t:
	for F in ${FILES}; do  ./ring $${F} ; done;


tv:
	for F in ${FILES}; do ${VALG} ./ring $${F} ; done;


tt:
	for F in ${FILES}; do  time ./ring $${F} ; done;