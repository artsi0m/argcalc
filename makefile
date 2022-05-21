# Makefile for BSD make
CFLAGS=-Wall -Wextra -g

argcalc:
	${CC} ${CFLAGS} $*.c -o $*
