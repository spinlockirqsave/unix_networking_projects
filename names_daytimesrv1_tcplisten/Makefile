# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_daytimesrv1_tcplisten

all:	${PROGS}
	
names_daytimesrv1_tcplisten:	main.o
		${CC} ${CFLAGS} -o names_daytimesrv1_tcplisten main.c networking_functions.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}