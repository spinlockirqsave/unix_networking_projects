/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 18, 2014, 04:19 PM
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
 * The sending process builds a msghdr structure
 * (Section 14.5) containing the descriptor. Then
 * sending process calls sendmsg to send the descriptor
 * across the Unix domain socket
 */
ssize_t
write_fd( int fd, void *ptr, size_t nbytes, int sendfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];

#ifdef	HAVE_MSGHDR_MSG_CONTROL
	union {
	  struct cmsghdr	cm;
	  char			control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *) CMSG_DATA(cmptr)) = sendfd;
#else
	msg.msg_accrights = (caddr_t) &sendfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;  // fill in optional data
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return( sendmsg( fd, &msg, 0));
}

ssize_t
Write_fd( int fd, void *ptr, size_t nbytes, int sendfd)
{
	ssize_t		n;

	if ( ( n = write_fd( fd, ptr, nbytes, sendfd)) < 0)
		err_sys( "write_fd error");

	return(n);
}

/*
 * This is sending process called by unix_domain_pass_descriptor
 */
int
main( int argc, char **argv)
{
	int		fd;

	if ( argc != 4)
		err_quit( "openfile <sockfd#> <filename> <mode>");

        /* open filename given by argv[2] as a descriptor to be sent */
	if ( ( fd = open( argv[2], atoi(argv[3]))) < 0)
		exit( ( errno > 0) ? errno : 255 );
        
        Write( fd, "written\n", 8);

        /* send opened fd descriptor through sockfd given by argv[1] */
	if ( write_fd( atoi(argv[1]), "", 1, fd) < 0)
		exit( ( errno > 0) ? errno : 255 );

	exit(0);
}