# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_connect

all:	${PROGS}
	
nonblock_connect:	main.o
		${CC} ${CFLAGS} -o nonblock_connect main.c networking_functions.c unixwrapper.c nonblock_connect.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}