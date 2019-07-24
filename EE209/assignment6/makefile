all:	ish

ish:	ish.o	imp.o
	gcc209	ish.o	imp.o	-o	ish

ish.o:	ish.c	ish.h
	gcc209	-c	-D_BSD_SOURCE	-D_GNU_SOURCE	ish.c

imp.o:	imp.c	ish.c
	gcc209	-c	-D_BSD_SOURCE	-D_GNU_SOURCE	imp.c

