# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = deamon_init

all:	${PROGS}
	
deamon_init:	main.o
		${CC} ${CFLAGS} -o deamon_init main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}