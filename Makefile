# Makefile for GNU MAKE
CFLAGS=-Wall -Wextra -g -lbsd

argcalc:
	${CC} ${CFLAGS} $@.c -o $@
