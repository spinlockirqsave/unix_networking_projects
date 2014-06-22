# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g -pthread -D_REENTRANT
LIBS+= -lpthread

PROGS = threads_err

all:	${PROGS}
	
threads_err:	main.o
		${CC} ${CFLAGS} -o threads_err main.c networking_functions.c unpthreads.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}