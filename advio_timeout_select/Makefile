# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = advio_timeout_select

all:	${PROGS}
	
advio_timeout_select:	main.o
		${CC} ${CFLAGS} -o advio_timeout_select main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}