/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 23, 2014, 01:59 PM
 * 
 * Nonblocking io prevents us from blocking while we could be doing
 * something productive
 * Nonblocking connects let us do other processing
 * while TCP's three-way handshake takes place,
 * instead of being blocked in the call to connect.
 * Unfortunately, these are also nonportable,
 * with different implementations having different
 * ways of indicating that the connection completed
 * successfully or encountered an error. 
 * 
 * to buffer: stdin  -> socket -> server ( toiptr, tooptr)
 * fr buffer: stdout <- socket <- server ( friptr, froptr)
 * 
 * tcpdump -i any -w tcpd tcp and port 9877
 * ./nonblock_tcpcli1 127.0.0.1 < file.txt > out 2> diag
 * tcpdump -r tcpd -N | sort diag -
 * stdin from file.txt or cuda_res.odt
 * stdout to out
 * stderr to diag
 * 
 * buffer:
 *                  optr                         iptr             MAXLINE
 * /-----------------/----------------------------/-----------------/
 * /  already sent   /         data to be sent    / available space /
 * 
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
            
		FD_ZERO( &rset);
		FD_ZERO( &wset);
                
		if ( stdineof == 0 && toiptr < &to[MAXLINE]) 
                    
                    /*
                     * there is a space in the "to" buffer, i.e.
                     * we can read from the stdin and store bytes in the buffer
                    */
                    FD_SET( STDIN_FILENO, &rset);	/* read from stdin */
                
		if ( friptr < &fr[MAXLINE])
                    
                    /*
                     * there is space in the "from" buffer, i.e
                     * we can read from the socket, store bytes in the buffer
                     */
                    FD_SET( sockfd, &rset);		/* read from socket */
                
		if ( tooptr != toiptr)
                    
                    /*
                     * we have bytes to be sent 
                     */
                    FD_SET( sockfd, &wset);		/* data to write to socket */
                
		if ( froptr != friptr)
                    
                    /*
                     * we have bytes to be written to stdout
                     */
                    FD_SET( STDOUT_FILENO, &wset);	/* data to write to stdout */

		Select( maxfdp1, &rset, &wset, NULL, NULL);

		if ( FD_ISSET( STDIN_FILENO, &rset)) {
                    
			if ( ( n = read( STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
                            
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
#ifdef VOL2
                                if( n >= 2) fprintf( stderr, "*(toiptr-2)=%d, *(toiptr-1)=%d, *(toiptr)=%d\n", *(toiptr-2),*(toiptr-1), *toiptr );
#endif
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
                    
			if ( ( nwritten = write( sockfd, tooptr, n)) < 0) {
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

/*

me@comp:projects/unix_networking_projects/nonblock_tcpcli1# ./nonblock_tcpcli1 127.0.0.1 < file.txt > out 2> diag

me@comp:projects/unix_networking_projects/nonblock_tcpcli1# tcpdump -i any -w tcpd tcp and port 9877
tcpdump: listening on any, link-type LINUX_SLL (Linux cooked), capture size 65535 bytes
^C10 packets captured
20 packets received by filter
0 packets dropped by kernel
 
me@comp:projects/unix_networking_projects/nonblock_tcpcli1# tcpdump -r tcpd -N | sort diag -
reading from file tcpd, link-type LINUX_SLL (Linux cooked)
20:14:50.384474 IP localhost.50811 > localhost.9877: Flags [S], seq 2347207189, win 32792, options [mss 16396,sackOK,TS val 109913530 ecr 0,nop,wscale 7], length 0
20:14:50.384502 IP localhost.9877 > localhost.50811: Flags [S.], seq 2885908781, ack 2347207190, win 32768, options [mss 16396,sackOK,TS val 109913530 ecr 109913530,nop,wscale 7], length 0
20:14:50.384524 IP localhost.50811 > localhost.9877: Flags [.], ack 1, win 257, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.384681: read 34 bytes from stdin
20:14:50.385082 IP localhost.50811 > localhost.9877: Flags [P.], seq 1:35, ack 1, win 257, options [nop,nop,TS val 109913530 ecr 109913530], length 34
20:14:50.385111: wrote 34 bytes to socket
20:14:50.385147: EOF on stdin
20:14:50.385175 IP localhost.50811 > localhost.9877: Flags [F.], seq 35, ack 1, win 257, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.385305 IP localhost.9877 > localhost.50811: Flags [.], ack 35, win 256, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.385367 IP localhost.9877 > localhost.50811: Flags [P.], seq 1:35, ack 36, win 256, options [nop,nop,TS val 109913530 ecr 109913530], length 34
20:14:50.385392 IP localhost.50811 > localhost.9877: Flags [.], ack 35, win 257, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.385416: read 34 bytes from socket
20:14:50.385420 IP localhost.9877 > localhost.50811: Flags [F.], seq 35, ack 36, win 256, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.385433 IP localhost.50811 > localhost.9877: Flags [.], ack 36, win 257, options [nop,nop,TS val 109913530 ecr 109913530], length 0
20:14:50.385489: wrote 34 bytes to stdout
20:14:50.385513: EOF on socket
 * 
 * 


 * bigger file:

 tcpdump -i any -w tcpd tcp and port 9877
 ^C15 packets captured
30 packets received by filter
0 packets dropped by kernel

 
 ./nonblock_tcpcli1 127.0.0.1 < cuda_res.odt > out 2> diag
 
me@comp:projects/unix_networking_projects/nonblock_tcpcli1# tcpdump -r tcpd -N | sort diag -
reading from file tcpd, link-type LINUX_SLL (Linux cooked)
21:03:55.095681 IP localhost.51241 > localhost.9877: Flags [S], seq 1631551149, win 32792, options [mss 16396,sackOK,TS val 110649707 ecr 0,nop,wscale 7], length 0
21:03:55.095725 IP localhost.9877 > localhost.51241: Flags [S.], seq 505625946, ack 1631551150, win 32768, options [mss 16396,sackOK,TS val 110649707 ecr 110649707,nop,wscale 7], length 0
21:03:55.095747 IP localhost.51241 > localhost.9877: Flags [.], ack 1, win 257, options [nop,nop,TS val 110649707 ecr 110649707], length 0
21:03:55.095821: read 4096 bytes from stdin
21:03:55.096195 IP localhost.51241 > localhost.9877: Flags [P.], seq 1:4097, ack 1, win 257, options [nop,nop,TS val 110649708 ecr 110649707], length 4096
21:03:55.096237: wrote 4096 bytes to socket
21:03:55.096239 IP localhost.9877 > localhost.51241: Flags [.], ack 4097, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 0
21:03:55.096274 IP localhost.9877 > localhost.51241: Flags [P.], seq 1:4097, ack 4097, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 4096
21:03:55.096275: read 4096 bytes from stdin
21:03:55.096290 IP localhost.51241 > localhost.9877: Flags [.], ack 4097, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 0
21:03:55.096309 IP localhost.51241 > localhost.9877: Flags [P.], seq 4097:8193, ack 4097, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 4096
21:03:55.096334: wrote 4096 bytes to socket
21:03:55.096348 IP localhost.9877 > localhost.51241: Flags [P.], seq 4097:8193, ack 8193, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 4096
21:03:55.096370: read 4096 bytes from stdin
21:03:55.096401: read 4096 bytes from socket
21:03:55.096448: wrote 4096 bytes to stdout
21:03:55.096476 IP localhost.51241 > localhost.9877: Flags [P.], seq 8193:12289, ack 8193, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 4096
21:03:55.096503: wrote 4096 bytes to socket
21:03:55.096533: read 4096 bytes from stdin
21:03:55.096560: read 4096 bytes from socket
21:03:55.096603: wrote 4096 bytes to stdout
21:03:55.096632: wrote 4096 bytes to socket
21:03:55.096663: read 1569 bytes from stdin
21:03:55.096686: wrote 1569 bytes to socket
21:03:55.096707: EOF on stdin
21:03:55.096733 IP localhost.51241 > localhost.9877: Flags [FP.], seq 12289:17954, ack 8193, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 5665
21:03:55.096776 IP localhost.9877 > localhost.51241: Flags [.], ack 17955, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 0
21:03:55.096805 IP localhost.9877 > localhost.51241: Flags [P.], seq 8193:12289, ack 17955, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 4096
21:03:55.096850 IP localhost.9877 > localhost.51241: Flags [FP.], seq 12289:17954, ack 17955, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 5665
21:03:55.096867 IP localhost.51241 > localhost.9877: Flags [.], ack 17955, win 256, options [nop,nop,TS val 110649708 ecr 110649708], length 0
21:03:55.096971: read 4096 bytes from socket
21:03:55.097025: wrote 4096 bytes to stdout
21:03:55.097054: read 4096 bytes from socket
21:03:55.097093: wrote 4096 bytes to stdout
21:03:55.097120: read 1569 bytes from socket
21:03:55.097158: wrote 1569 bytes to stdout
21:03:55.097182: EOF on socket

 */