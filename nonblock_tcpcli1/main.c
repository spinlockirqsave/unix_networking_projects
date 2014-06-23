/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 20, 2014, 10:59 PM
 * 
 * Nonblocking io prevents us from blocking while we could be doing
 * something productive
 * 
 * to buffer: stdin  -> socket -> server ( toiptr, tooptr)
 * fr buffer: stdout <- socket <- server ( friptr, froptr)
 * 
 * tcpdump -w tcpd tcp and port 9877
 * ./nonblock_tcpcli1 127.0.0.1 < file.txt > out 2> diag
 * stdin from file.txt
 * stdout to out
 * stderr to diag
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"

#define VOL2

void
str_cli( FILE *fp, int sockfd)
{
	int		maxfdp1, val, stdineof;
	ssize_t		n, nwritten;
	fd_set		rset, wset;
	char		to[MAXLINE], fr[MAXLINE];
	char		*toiptr, *tooptr, *friptr, *froptr;

	val = Fcntl( sockfd, F_GETFL, 0);
	Fcntl( sockfd, F_SETFL, val | O_NONBLOCK);

	val = Fcntl( STDIN_FILENO, F_GETFL, 0);
	Fcntl( STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

	val = Fcntl( STDOUT_FILENO, F_GETFL, 0);
	Fcntl( STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	toiptr = tooptr = to;	/* initialize buffer pointers */
	friptr = froptr = fr;
	stdineof = 0;

	maxfdp1 = max( max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
	for ( ; ; ) {
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if ( stdineof == 0 && toiptr < &to[MAXLINE])
			FD_SET( STDIN_FILENO, &rset);	/* read from stdin */
		if ( friptr < &fr[MAXLINE])
			FD_SET( sockfd, &rset);			/* read from socket */
		if ( tooptr != toiptr)
			FD_SET( sockfd, &wset);			/* data to write to socket */
		if ( froptr != friptr)
			FD_SET( STDOUT_FILENO, &wset);	/* data to write to stdout */

		Select ( maxfdp1, &rset, &wset, NULL, NULL);

		if ( FD_ISSET( STDIN_FILENO, &rset)) {
			if ( ( n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
				if ( errno != EWOULDBLOCK)
					err_sys( "read error on stdin");

			} else if ( n == 0) {
#ifdef	VOL2
				fprintf( stderr, "%s: EOF on stdin\n", gf_time());
#endif
				stdineof = 1;			/* all done with stdin */
				if ( tooptr == toiptr)
					Shutdown( sockfd, SHUT_WR);/* send FIN */

			} else {
#ifdef	VOL2
				fprintf( stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
#endif
				toiptr += n;			/* # just read */
				FD_SET( sockfd, &wset);	/* try and write to socket below */
			}
		}

		if ( FD_ISSET( sockfd, &rset)) {
			if ( ( n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
				if ( errno != EWOULDBLOCK)
					err_sys( "read error on socket");

			} else if ( n == 0) {
#ifdef	VOL2
				fprintf( stderr, "%s: EOF on socket\n", gf_time());
#endif
				if ( stdineof)
					return;		/* normal termination */
				else
					err_quit( "str_cli: server terminated prematurely");

			} else {
#ifdef	VOL2
				fprintf( stderr, "%s: read %d bytes from socket\n",
								gf_time(), n);
#endif
				friptr += n;		/* # just read */
				FD_SET( STDOUT_FILENO, &wset);	/* try and write below */
			}
		}

		if ( FD_ISSET( STDOUT_FILENO, &wset) && ( ( n = friptr - froptr) > 0)) {
			if ( ( nwritten = write( STDOUT_FILENO, froptr, n)) < 0) {
				if ( errno != EWOULDBLOCK)
					err_sys("write error to stdout");

			} else {
#ifdef	VOL2
				fprintf( stderr, "%s: wrote %d bytes to stdout\n",
								gf_time(), nwritten);
#endif
				froptr += nwritten;		/* # just written */
				if ( froptr == friptr)
					froptr = friptr = fr;	/* back to beginning of buffer */
			}
		}

		if ( FD_ISSET( sockfd, &wset) && ( ( n = toiptr - tooptr) > 0)) {
			if ( ( nwritten = write(sockfd, tooptr, n)) < 0) {
				if ( errno != EWOULDBLOCK)
					err_sys( "write error to socket");

			} else {
#ifdef	VOL2
				fprintf( stderr, "%s: wrote %d bytes to socket\n",
								gf_time(), nwritten);
#endif
				tooptr += nwritten;	/* # just written */
				if ( tooptr == toiptr) {
					toiptr = tooptr = to;	/* back to beginning of buffer */
					if (stdineof)
						Shutdown( sockfd, SHUT_WR);	/* send FIN */
				}
			}
		}
	}
}

/*
 * 
 */
int
main( int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_in	servaddr;

	if ( argc != 2)
		err_quit( "usage: tcpcli <IPaddress>");

	sockfd = Socket( AF_INET, SOCK_STREAM, 0);

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	Inet_pton( AF_INET, argv[1], &servaddr.sin_addr);

	Connect( sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_cli( stdin, sockfd);		/* do it all */

	exit(0);
}