include Make.defines

PROGS =	server2_sum

all:	${PROGS}

server2_sum:	main.o
		${CC} ${CFLAGS} -o $@ main.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
