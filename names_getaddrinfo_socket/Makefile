# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_getaddrinfo_socket

all:	${PROGS}
	
names_getaddrinfo_socket:	main.o
		${CC} ${CFLAGS} -o names_getaddrinfo_socket main.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}