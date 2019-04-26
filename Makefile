# Makefile for IOS project 2
# Author: Igor Ignác xignac00@fit.vutbr.cz
# VUT FIT 2017-208

CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -lpthread
DEPS= proj2.h
OBJ= proj2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

proj2: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f proj2 *.o
