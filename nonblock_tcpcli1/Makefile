# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_tcpli1

all:	${PROGS}
	
nonblock_tcpli1:	main.o
		${CC} ${CFLAGS} -o nonblock_tcpli1 main.c networking_functions.c unixwrapper.c gf_time.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}