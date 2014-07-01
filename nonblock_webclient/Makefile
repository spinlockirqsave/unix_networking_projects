# Environment 
MKDIR=mkdir
CC= gcc
CFLAGS= -g
LIBS+= -lpthread

PROGS = nonblock_webclient

all:	${PROGS}
	
nonblock_webclient:	networking_functions.o home_page.o start_connect.o write_get_cmd.o host_serv.o
		${CC} ${CFLAGS} -o $@ networking_functions.o main.o home_page.o start_connect.o \
			write_get_cmd.o host_serv.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}