# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = unix_domain_bind

all:	${PROGS}
	
unix_domain_bind:	main.o
		${CC} ${CFLAGS} -o unix_domain_bind main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c ${LIBS}
	

clean:
		rm -f ${PROGS} ${CLEANFILES}