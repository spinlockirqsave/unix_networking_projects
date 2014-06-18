# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = unix_domain_pass_decriptor

all:	${PROGS}
	
unix_domain_pass_decriptor:	main.o
		${CC} ${CFLAGS} -o unix_domain_pass_decriptor main.c networking_functions.c unixwrapper.c sock_ntop_host.c deamon_init.c readable_timeout.c read_fd.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}