all:	testheapmgrgnu	testheapmgrkr	testheapmgrbase	testheapmgr1	testheapmgr2

testheapmgrgnu:	heapmgrgnu.o	testheapmgr.o
	gcc209 heapmgrgnu.o	testheapmgr.o	-o	testheapmgrgnu
heapmgrgnu.o:	heapmgrgnu.c	heapmgr.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	heapmgrgnu.c


testheapmgrkr:	heapmgrkr.o	testheapmgr.o
	gcc209	heapmgrkr.o	testheapmgr.o	-o	testheapmgrkr
heapmgrkr.o:	heapmgrkr.c	heapmgr.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	heapmgrkr.c


testheapmgrbase:	heapmgrbase.o	chunkbase.o	testheapmgr.o
	gcc209	heapmgrbase.o	chunkbase.o	testheapmgr.o	-o	testheapmgrbase
heapmgrbase.o:	heapmgrbase.c	chunkbase.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	heapmgrbase.c
chunkbase.o:	chunkbase.c	chunkbase.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	chunkbase.c


testheapmgr1:	heapmgr1.o	chunk1.o	testheapmgr.o
	gcc209	heapmgr1.o	chunk1.o	testheapmgr.o	-o	testheapmgr1
heapmgr1.o:	heapmgr1.c	chunkbase.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	heapmgr1.c
chunk1.o:	chunk1.c	chunkbase.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	chunk1.c


testheapmgr2:	heapmgr2.o	chunk2.o	testheapmgr.o
	gcc209	heapmgr2.o	chunk2.o	testheapmgr.o	-o	testheapmgr2
heapmgr2.o:	heapmgr2.c	chunk2.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	heapmgr2.c
chunk2.o:	chunk2.c	chunk2.h
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	chunk2.c


testheapmgr.o:	testheapmgr.c
	gcc209	-c	-O3	-D	NDEBUG	-std=gnu99	testheapmgr.c

