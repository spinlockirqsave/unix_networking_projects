# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = ibclient_4

all:	${PROGS}
	
ibclient_4:	main.o
		${CC} ${CFLAGS} -o ibclient_4 main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}