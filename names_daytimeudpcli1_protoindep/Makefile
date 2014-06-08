# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_daytimeudpcli1_protoindep

all:	${PROGS}
	
names_daytimeudpcli1_protoindep:	main.o
		${CC} ${CFLAGS} -o names_daytimeudpcli1_protoindep main.c networking_functions.c unixwrapper.c sock_ntop_host.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}