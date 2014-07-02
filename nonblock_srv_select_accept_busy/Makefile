# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_srv_select_accept_busy

all:	${PROGS}
	
nonblock_srv_select_accept_busy:	main.o
		${CC} ${CFLAGS} -o nonblock_srv_select_accept_busy main.c networking_functions.c unixwrapper.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}