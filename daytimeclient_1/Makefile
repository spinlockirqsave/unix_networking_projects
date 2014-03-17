include Make.defines

PROGS =	daytimeclient_1.out

all:	${PROGS}

daytimeclient_1.out:	daytimeclient_1.o
		${CC} ${CFLAGS} -o $@ daytimeclient_1.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
