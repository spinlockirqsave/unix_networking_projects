# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_tcpcli3_rst

all:	${PROGS}
	
nonblock_tcpcli3_rst:	main.o
		${CC} ${CFLAGS} -o nonblock_tcpcli3_rst main.c networking_functions.c unixwrapper.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}