/* 
 * File:   networking_functions.h
 * Author: piter
 *
 * Created on June 2, 2014, 3:21 PM
 */

#ifndef NETWORKING_FUNCTIONS_H
#define	NETWORKING_FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

/* All data returned by the network data base library are supplied in
   host order and returned in network order (suitable for use in
   system calls).  */
#include <netdb.h>

#include <errno.h>
#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include	<sys/types.h>
#include        <sys/wait.h>            /* for waitpid */
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#include <math.h>

#include <fcntl.h>
#include <poll.h>

#include <limits.h>		/* for open_max() */
#include <sys/un.h>             /* for AF_UNIX, struct sockaddr_un */

/* POSIX requires that an #include of <poll.h> DefinE INFTIM, but many
   systems still DefinE it in <sys/stropts.h>.  We don't want to include
   all the STREAMS stuff if it's not needed, so we just DefinE INFTIM here.
   This is the standard value, but there's no guarantee it is -1. */

#define INFTIM          (-1)    /* infinite poll timeout */
#define	HAVE_POLL

/* 
 * we do have msghdr.msg_control on Ubuntu. POSIX specifies that the descriptor be sent
 * as ancillary data (the msg_control member of the msghdr structure, Section 14.6),
 * but older implementations use the msg_accrights member.
*/
#define HAVE_MSGHDR_MSG_CONTROL

#ifdef  OPEN_MAX
static long openmax = open_max();
#else
static long openmax = 0;
#endif

/*
 * If open_max() is indeterminate, we're not
 * guaranteed that this is adequate.
 */
#define OPEN_MAX_GUESS  256

#define MAXLINE 4096            /* max text linr lrngth */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */
#define SERV_PORT 9877
#define SA struct sockaddr
int		daemon_proc;		/* set nonzero by daemon_init() */

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#define	LISTENQ		1024	/* 2nd argument to listen() */

typedef void (Sigfunc)(int);    /* signal handler */

static void	err_doit(int, int, const char *, va_list);

void
err_sys( const char *fmt, ...);

void
err_quit(const char *fmt, ...);

void
err_msg(const char *fmt, ...);

void
err_ret(const char *fmt, ...);

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

void
Close( int fd);

void
Listen(int fd, int backlog);

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */

#ifdef	__cplusplus
}
#endif

#endif	/* NETWORKING_FUNCTIONS_H */

