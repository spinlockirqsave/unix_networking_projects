CC=g++
CFLAGS+= -g
LIBS= -lsctp
PROGS = sctpcli_1

all:	${PROGS}

sctpcli_1:	main.o
		${CC} ${CFLAGS} -o $@ main.c ${LIBS}