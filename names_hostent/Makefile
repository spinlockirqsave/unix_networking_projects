# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_hostent

all:	${PROGS}
	
names_hostent:	main.o
		${CC} ${CFLAGS} -o names_hostent main.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}
