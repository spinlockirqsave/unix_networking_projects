/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 18, 2014, 03:17 PM
 * 
 * A Unix domain socket or IPC socket (inter-process communication socket)
 * is a data communications endpoint for exchanging data between processes
 * executing within the same host operating system. While similar in
 * functionality to named pipes, Unix domain sockets may be created
 * as connection‑mode (SOCK_STREAM or SOCK_SEQPACKET) or as connectionless
 * (SOCK_DGRAM), while pipes are streams only. Processes using Unix domain
 * sockets do not need to share a common ancestry.
 * 
 * Unix domain sockets use the file system as their address name space.
 * They are referenced by processes as inodes in the file system.
 * This allows two processes to open the same socket in order to communicate.
 * However, communication occurs entirely within the operating system kernel.
 * In addition to sending data, processes may send file descriptors across
 * a Unix domain socket connection using the sendmsg() and recvmsg() system
 * calls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"




int
my_open( const char *pathname, int mode)
{
	int		fd, sockfd[2], status;
	pid_t		childpid;
	char		c, argsockfd[10], argmode[10];

        /* create a stream pipe that can be used to exchange the descriptor */
	Socketpair( AF_LOCAL, SOCK_STREAM, 0, sockfd);

	if ( ( childpid = Fork()) == 0) {		/* child process */
		Close(sockfd[0]);
		snprintf( argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
		snprintf( argmode, sizeof(argmode), "%d", mode);
                
                /* 
                 * This sending process unix_domain_openfile builds a msghdr structure
                 * (Section 14.5) containing the descriptor. Then sending process calls
                 * sendmsg to send the descriptor across the Unix domain socket sockfd[1]
                 * passed in argsockfd
                 */
		execl( "./unix_domain_openfile", "openfile",
                        argsockfd, pathname, argmode, (char *) NULL);
		err_sys( "execl error");
	}

	/* 
         * parent process - wait for the child to terminate,
         * close the end we don't use - it has been passed already
         * to the unix_domain_openfile
         */
	Close( sockfd[1]);
	Waitpid( childpid, &status, 0);
        
	if ( WIFEXITED(status) == 0)
		err_quit( "child did not terminate");
	if ( ( status = WEXITSTATUS(status)) == 0)
		Read_fd( sockfd[0], &c, 1, &fd);
	else {
		errno = status;		/* set errno value from child's status */
		fd = -1;
	}

	Close( sockfd[0]);
	return(fd);
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int		fd, n;
	char	buff[BUFFSIZE];

	if (argc != 2)
		err_quit("usage: mycat <pathname>");

	if ( (fd = my_open( argv[1], O_RDONLY)) < 0)
		err_sys("cannot open %s", argv[1]);

	while ( (n = Read(fd, buff, BUFFSIZE)) > 0)
		Write(STDOUT_FILENO, buff, n);

	exit(0);
}