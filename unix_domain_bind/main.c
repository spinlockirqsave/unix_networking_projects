/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 11, 2014, 10:58 PM
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




/*
 * 
 */
int
main(int argc, char **argv)
{
	int					sockfd;
	socklen_t			len;
	struct sockaddr_un	addr1, addr2;

	if ( argc != 2)
		err_quit( "usage: unixbind <pathname>");

        /* Create IPC socket */
	sockfd = Socket( AF_LOCAL, SOCK_STREAM, 0);

	unlink(argv[1]);		/* OK if this fails */

	bzero( &addr1, sizeof(addr1));
	addr1.sun_family = AF_LOCAL;
	strncpy( addr1.sun_path, argv[1], sizeof(addr1.sun_path)-1);
	Bind( sockfd, (SA *) &addr1, SUN_LEN(&addr1));

	len = sizeof(addr2);
	Getsockname( sockfd, (SA *) &addr2, &len);
	printf( "bound name = %s, returned len = %d\n", addr2.sun_path, len);
	
	exit(0);
}