include Make.defines

PROGS =	server_1.out

all:	${PROGS}

server_1.out:	server_1.o
		${CC} ${CFLAGS} -o $@ server_1.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
