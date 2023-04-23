CC = gcc
LFLAGS = -lncurses

all: test

test: test.o stack.o cursor.o history.o
	$(CC) -Wall -o $@ $^ $(LFLAGS)

test.o: test.c stack.h cursor.h history.h

stack.o: test.c stack.h

cursor.o : cursor.c cursor.h

history.o : history.c history.h


clean:
	@echo "Clean..."
	@rm -f *.out *.o