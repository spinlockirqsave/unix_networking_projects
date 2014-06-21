# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g
LIBS+= -lpthread

PROGS = threads_srv1

all:	${PROGS}
	
threads_srv1:	main.o
		${CC} ${CFLAGS} -o threads_srv1 main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c unpthreads.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}