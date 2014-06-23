# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_tcpcli1

all:	${PROGS}
	
nonblock_tcpcli1:	main.o
		${CC} ${CFLAGS} -o nonblock_tcpcli1 main.c networking_functions.c unixwrapper.c gf_time.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}