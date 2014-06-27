# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_tcpcli2

all:	${PROGS}
	
nonblock_tcpcli2:	main.o
		${CC} ${CFLAGS} -o nonblock_tcpcli2 main.c networking_functions.c unixwrapper.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}