/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 18, 2014, 10:58 AM
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
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( ( pid = waitpid( -1, &stat, WNOHANG)) > 0) {
		printf( "child %d terminated\n", pid);
	}
	return;
}

/* same as before (same as in the case of AF_INET socket) */
void
str_echo( int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE];

again:
	while ( ( n = read( sockfd, buf, MAXLINE)) > 0)
		Writen( sockfd, buf, n);

	if ( n < 0 && errno == EINTR)
		goto again;
	else if ( n < 0)
		err_sys( "str_echo: read error");
}

/*
 * 
 */
int
main( int argc, char **argv)
{
	int			listenfd, connfd, clilen;
	socklen_t		len, childpid;
	struct sockaddr_un	servaddr, addr2;
        struct sockaddr_un	cliaddr;

	if ( argc != 2)
		err_quit( "usage: unix_domain_srv1 <pathname>");

        /* Create IPC socket */
	listenfd = Socket( AF_LOCAL, SOCK_STREAM, 0);

        /* unlink in case the link exists from an earlier run of the server */
	unlink( argv[1]);		/* OK if this fails */

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy( servaddr.sun_path, argv[1], sizeof(servaddr.sun_path)-1);
	Bind( listenfd, (SA *) &servaddr, SUN_LEN(&servaddr));

	len = sizeof( addr2);
	Getsockname( listenfd, (SA *) &addr2, &len);
	printf( "bound name = %s, returned len = %d\n", addr2.sun_path, len);
	
	Listen( listenfd, LISTENQ);

	Signal( SIGCHLD, sig_chld);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ( ( connfd = accept( listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if ( errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys( "accept error");
		}

		if ( ( childpid = Fork()) == 0) {	/* child process */
			Close( listenfd);               /* close listening socket */
			str_echo( connfd);              /* process request */
			exit( 0);
		}
                
		Close( connfd);                         /* parent closes connected socket */
	}
}