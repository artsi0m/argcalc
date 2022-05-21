# Makefile for GNU MAKE
CFLAGS=-Wall -Wextra -g

argcalc:
	${CC} ${CFLAGS} $@.c -o $@
