# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = threads_srv2

all:	${PROGS}
	
threads_srv2:	main.o
		${CC} ${CFLAGS} -o threads_srv2 main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c unpthreads.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}