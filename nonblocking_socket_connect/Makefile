# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = nonblocking_socket_connect

all:	${PROGS}
	
nonblocking_socket_connect:	main.o
		${CC} ${CFLAGS} -o nonblocking_socket_connect main.c networking_functions.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}