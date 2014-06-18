/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 16, 2014, 11:25 PM
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

void
str_cli( FILE *fp, int sockfd)
{
	int		maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	int		n;

	stdineof = 0;
	FD_ZERO(&rset);
	for ( ; ; ) {
		if ( stdineof == 0)
			FD_SET( fileno(fp), &rset);
		FD_SET( sockfd, &rset);
		maxfdp1 = max( fileno(fp), sockfd) + 1;
		Select( maxfdp1, &rset, NULL, NULL, NULL);

		if ( FD_ISSET( sockfd, &rset)) {	/* socket is readable */
			if ( ( n = Read( sockfd, buf, MAXLINE)) == 0) {
				if ( stdineof == 1)
					return;		/* normal termination */
				else
					err_quit( "str_cli: server terminated prematurely");
			}

			Write( fileno(stdout), buf, n);
		}

		if ( FD_ISSET( fileno(fp), &rset)) {  /* input is readable */
			if ( ( n = Read( fileno(fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				Shutdown( sockfd, SHUT_WR);	/* send FIN */
				FD_CLR( fileno(fp), &rset);
				continue;
			}

			Writen( sockfd, buf, n);
		}
	}
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_un	servaddr;
        
        if ( argc != 2)
		err_quit( "usage: unix_domain_cli1 <pathname>");

        /* Create IPC socket */
	sockfd = Socket( AF_LOCAL, SOCK_STREAM, 0);

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy( servaddr.sun_path, argv[1], sizeof(servaddr.sun_path)-1);

	Connect( sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_cli( stdin, sockfd);		/* do it all */

	exit(0);
}