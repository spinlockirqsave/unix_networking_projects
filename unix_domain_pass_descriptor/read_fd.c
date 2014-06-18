
#include "networking_functions.h"

/* 
 * The client and server must have some application protocol so that the receiver
 * of the descriptor knows when to expect it. If the receiver calls recvmsg without
 * allocating room to receive the descriptor, and a descriptor was passed and is
 * ready to be read, the descriptor that was being passed is closed (p. 518 of TCPv2).
 * Also, the MSG_PEEK flag should be avoided with recvmsg if a descriptor is expected,
 * as the result is unpredictable.
 */
ssize_t
read_fd( int fd, void *ptr, size_t nbytes, int *recvfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	ssize_t		n;

        /* 
        * we do have msghdr.msg_control on Ubuntu. POSIX specifies that the descriptor be sent
        * as ancillary data (the msg_control member of the msghdr structure, Section 14.6),
        * but older implementations use the msg_accrights member.
        */
#ifdef	HAVE_MSGHDR_MSG_CONTROL
        
        /* helper */
	union {
	  struct cmsghdr	cm;
	  char			control[CMSG_SPACE(sizeof(int))];
	} control_un;
        
	struct cmsghdr	*cmptr;

        /* assign char* to void* */
	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
#else
        /* older implementations */
	int     newfd;
	msg.msg_accrights = (caddr_t) &newfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

        /* receive the descriptor on the Unix domain socket fd.
         * It is normal for the descriptor number in the receiving process
         * to differ from the descriptor number in the sending process. Passing
         * a descriptor is not passing a descriptor number, but involves creating
         * a new descriptor in the receiving process that refers to the same file
         * table entry within the kernel as the descriptor that was sent by
         * the sending process.
*/
	if ( ( n = recvmsg( fd, &msg, 0)) <= 0)
		return(n);

#ifdef	HAVE_MSGHDR_MSG_CONTROL

	if ( ( cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
	    cmptr->cmsg_len == CMSG_LEN(sizeof(int)))
        {
		if ( cmptr->cmsg_level != SOL_SOCKET)
			err_quit( "control level != SOL_SOCKET");
                
		if ( cmptr->cmsg_type != SCM_RIGHTS)
			err_quit( "control type != SCM_RIGHTS");
                
                /* OK, copy descriptor*/
		*recvfd = *((int *) CMSG_DATA(cmptr));
	} else
		*recvfd = -1;		/* descriptor was not passed */
#else

	if (msg.msg_accrightslen == sizeof(int))
		*recvfd = newfd;
	else
		*recvfd = -1;		/* descriptor was not passed */
#endif

	return(n);
}
/* end read_fd */

ssize_t
Read_fd( int fd, void *ptr, size_t nbytes, int *recvfd)
{
	ssize_t		n;

	if ( ( n = read_fd( fd, ptr, nbytes, recvfd)) < 0)
		err_sys( "read_fd error");

	return(n);
}
