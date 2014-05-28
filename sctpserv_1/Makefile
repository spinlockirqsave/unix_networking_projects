CC=g++
CFLAGS+= -g
LIBS= -lsctp
PROGS = sctpserv_1

all:	${PROGS}

sctpserv_1:	main.o
		${CC} ${CFLAGS} -o $@ main.cpp ${LIBS}
	