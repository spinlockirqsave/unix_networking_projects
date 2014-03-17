include Make.defines

PROGS =	ibclient_1.out

all:	${PROGS}

ibclient_1.out:	main.o
		${CC} ${CFLAGS} -o $@ main.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
