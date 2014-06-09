# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_daytimeudpsrv2_unconnected_protoindep

all:	${PROGS}
	
names_daytimeudpsrv2_unconnected_protoindep:	main.o
		${CC} ${CFLAGS} -o names_daytimeudpsrv2_unconnected_protoindep main.c networking_functions.c unixwrapper.c sock_ntop_host.c ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}