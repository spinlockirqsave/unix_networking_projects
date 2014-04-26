include Make.defines

PROGS =	ibclient_2.out

all:	${PROGS}

ibclient_2.out:	main.o
		${CC} ${CFLAGS} -o $@ main.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
