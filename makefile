CC = gcc

CFLAGS = -c -Wall -I

CLEANUP = rm -rf *o myshell

all:
	gcc -o myshell myshell.c utility.c

clean:
	$(CLEANUP)
