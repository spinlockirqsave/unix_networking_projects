include Make.defines

PROGS =	server_1

all:	${PROGS}

server_1:	server_1.o
		${CC} ${CFLAGS} -o $@ server_1.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
