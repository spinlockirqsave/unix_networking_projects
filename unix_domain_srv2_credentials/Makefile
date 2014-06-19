# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = unix_domain_srv2_credentials

all:	${PROGS}
	
unix_domain_srv2_credentials:	main.o
		${CC} ${CFLAGS} -o unix_domain_srv2_credentials main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c read_fd.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}