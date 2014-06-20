# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = unix_domain_srv3_udp

all:	${PROGS}
	
unix_domain_srv3_udp:	main.o
		${CC} ${CFLAGS} -o unix_domain_srv3_udp main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}