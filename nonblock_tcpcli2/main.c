/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 27, 2014, 18:12 PM
 * 
 * TCP connection is full-duplex, in this example the parent
 * and child are sharing the same socket descriptor: The parent
 * writes to the socket and the child reads from the socket.
 * There is only one socket, one socket receive buffer,
 * and one socket send buffer, but this socket is referenced
 * by two descriptors: one in the parent and one in the child.
 * We again need to worry about the termination sequence. Normal
 * termination occurs when the EOF on standard input is encountered.
 * The parent reads this EOF and calls shutdown to send a FIN. (The parent
 * cannot call close because there still might be something to read
 * from the server on the way to the socket) But when this happens,
 * the child needs to continue copying from the server to the standard
 * output, until it reads an EOF on the socket. It is also possible
 * for the server process to terminate prematurely; if this occurs,
 * the child will read an EOF on the socket. If this happens, the child
 * must tell the parent to stop copying from the standard input to
 * the socket, the child sends the SIGTERM signal to the parent,
 * in case the parent is still running. Another way to handle this would
 * be for the child to terminate and have the parent catch SIGCHLD,
 * if the parent is still running.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"

#define VOL2

void
str_cli( FILE *fp, int sockfd)
{
	pid_t	pid;
	char	sendline[MAXLINE], recvline[MAXLINE];

	if ( ( pid = Fork()) == 0) {		/* child: server -> stdout */
		while ( Read( sockfd, recvline, MAXLINE) > 0)
			Fputs( recvline, stdout);

		kill( getppid(), SIGTERM);	/* in case parent still running */
		exit(0);
	}

		/* parent: stdin -> server */
	while ( Fgets(sendline, MAXLINE, fp) != NULL)
		Writen( sockfd, sendline, strlen(sendline));

	Shutdown( sockfd, SHUT_WR);	/* EOF on stdin, send FIN */
	pause();
	return;
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