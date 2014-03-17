include Make.defines

PROGS =	daytimeserver_1.out

all:	${PROGS}

daytimeserver_1.out:	daytimeserver_1.o
		${CC} ${CFLAGS} -o $@ daytimeserver_1.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
