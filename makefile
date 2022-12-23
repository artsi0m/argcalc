# Makefile for BSD make
CFLAGS=-Wall -Wextra -g

PROG	= argcalc
SRCS	= argcalc.c
MAN	=
.include <bsd.prog.mk>
