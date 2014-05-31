# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS+= -g


PROGS = names_hostent2

all:	${PROGS}
	
names_hostent2:	main.o
		${CC} ${CFLAGS} -o names_hostent2 main.c ${LIBS}
	


clean:
		rm -f ${PROGS} ${CLEANFILES}
