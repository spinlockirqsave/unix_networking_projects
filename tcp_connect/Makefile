# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = tcp_connect

all:	${PROGS}
	
tcp_connect:	main.o
		${CC} ${CFLAGS} -o tcp_connect main.c networking_functions.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}