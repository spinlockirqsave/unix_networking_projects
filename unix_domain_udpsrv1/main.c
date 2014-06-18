/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 18, 2014, 00:52 PM
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


/* same as before (same as in the case of AF_INET socket) */
void
dg_echo( int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	for ( ; ; ) {
		len = clilen;
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		Sendto(sockfd, mesg, n, 0, pcliaddr, len);
	}
}

/*
 * 
 */
int
main( int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_un	servaddr, cliaddr;

	if ( argc != 2)
		err_quit( "usage: unix_domain_udpsrv1 <pathname>");

        /* Create IPC socket */
	sockfd = Socket( AF_LOCAL, SOCK_DGRAM, 0);

        /* unlink in case the link exists from an earlier run of the server */
	unlink( argv[1]);		/* OK if this fails */

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy( servaddr.sun_path, argv[1], sizeof(servaddr.sun_path)-1);
	Bind( sockfd, (SA *) &servaddr, SUN_LEN(&servaddr));

	dg_echo( sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}