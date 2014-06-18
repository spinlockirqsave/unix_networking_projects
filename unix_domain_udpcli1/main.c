/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 16, 2014, 13:30:20 PM
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
dg_cli( FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while ( Fgets( sendline, MAXLINE, fp) != NULL) {

		Sendto( sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		n = Recvfrom( sockfd, recvline, MAXLINE, 0, NULL, NULL);

		recvline[n] = 0;	/* null terminate */
		Fputs( recvline, stdout);
	}
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_un	cliaddr, servaddr;
        
        if ( argc != 2)
		err_quit( "usage: unix_domain_cli1 <pathname>");

        /* Create IPC socket */
	sockfd = Socket( AF_LOCAL, SOCK_DGRAM, 0);

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy( servaddr.sun_path, argv[1], sizeof(servaddr.sun_path)-1);

	bzero( &cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;
	strcpy( cliaddr.sun_path, tmpnam(NULL));

	Bind( sockfd, (SA *) &cliaddr, sizeof(cliaddr));

	dg_cli( stdin, sockfd, (SA *) &servaddr, sizeof(servaddr)); /* do it all? */

	exit(0);
}