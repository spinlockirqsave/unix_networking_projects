# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = advio_timeout_sigalrm

all:	${PROGS}
	
advio_timeout_sigalrm:	main.o
		${CC} ${CFLAGS} -o advio_timeout_sigalrm main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}