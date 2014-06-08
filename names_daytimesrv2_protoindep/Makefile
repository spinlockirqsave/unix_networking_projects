# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_daytimesrv2_protoindep

all:	${PROGS}
	
names_daytimesrv2_protoindep:	main.o
		${CC} ${CFLAGS} -o names_daytimesrv2_protoindep main.c networking_functions.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}