/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on May 15, 2014, 6:08 PM
 * 
 * This server avoids all the overhead of creating a new process
 * for each client and it is a nice example of pselect. Nevertheless,
 * there is a problem with this server that can be easily fixed
 * by making the listening socket nonblocking and then checking
 * for, and ignoring, a few errors from accept
 * ( deliberately not fixed here as an example)
 * 
 * We block SIGINT.
 * When pselect is called, it replaces the signal mask of
 * the process with an empty set (i.e., zeromask) and
 * then checks the descriptors, possibly going to sleep.
 * But when pselect returns, the signal mask of the
 * process is reset to its value before pselect was
 * called (i.e., SIGINT is blocked).
 * 
 * POSIX defines the function pselect, which increases the time precision from microseconds to
 * nanoseconds and takes a new argument that is a pointer to a signal set. This lets us avoid
 * race conditions when signals are being caught and we talk more about in next examples
 */

#include <stdio.h>
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

#define MAXLINE 4096
#define SERV_PORT 9877
#define SA struct sockaddr
int		daemon_proc;		/* set nonzero by daemon_init() */

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#define	LISTENQ		1024	/* 2nd argument to listen() */

typedef void (Sigfunc)(int);    /* signal handler */

/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */
static void
err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int		errno_save, n;
	char	buf[MAXLINE + 1];

	errno_save = errno;		/* value caller might want printed */
#ifdef	HAVE_VSNPRINTF
	vsnprintf(buf, MAXLINE, fmt, ap);	/* safe */
#else
	vsprintf(buf, fmt, ap);					/* not safe */
#endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
	strcat(buf, "\n");

	if (daemon_proc) {
		syslog(level, buf);
	} else {
		fflush(stdout);		/* in case stdout and stderr are the same */
		fputs(buf, stderr);
		fflush(stderr);
	}
	return;
}

/* Fatal error unrelated to system call
 * Print message and terminate */

void
err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Fatal error related to system call
 * Print message and terminate */

void
err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

void
Inet_pton(int family, const char *strptr, void *addrptr)
{
	int		n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		err_sys("inet_pton error for %s", strptr);	/* errno set */
	else if (n == 0)
		err_quit("inet_pton error for %s", strptr);	/* errno not set */

	/* nothing to return */
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}

int
Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0)
		err_sys("socket error");
	return(n);
}

void
Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0)
		err_sys("connect error");
}

ssize_t
Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1)
		err_sys("read error");
	return(n);
}

void
Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes)
		err_sys("write error");
}

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = ( const char*) vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
	int		n;

	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		err_sys("select error");
	return(n);		/* can return 0 on timeout */
}

void
Send(int fd, const void *ptr, size_t nbytes, int flags)
{
	if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes)
		err_sys("send error");
}

void
Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
		err_sys("sendto error");
}

void
Sendmsg(int fd, const struct msghdr *msg, int flags)
{
	unsigned int	i;
	ssize_t			nbytes;

	nbytes = 0;	/* must first figure out what return value should be */
	for (i = 0; i < msg->msg_iovlen; i++)
		nbytes += msg->msg_iov[i].iov_len;

	if (sendmsg(fd, msg, flags) != nbytes)
		err_sys("sendmsg error");
}

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0)
		err_sys("setsockopt error");
}

void
Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		err_sys("shutdown error");
}

char *
Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		err_sys("fgets error");

	return (rptr);
}

FILE *
Fopen(const char *filename, const char *mode)
{
	FILE	*fp;

	if ( (fp = fopen(filename, mode)) == NULL)
		err_sys("fopen error");

	return(fp);
}

void
Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		err_sys("fputs error");
}

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			err_sys("accept error");
	}
	return(n);
}

void
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		err_sys("bind error");
}

void
Close(int fd)
{
	if (close(fd) == -1)
		err_sys("close error");
}

int
Fcntl(int fd, int cmd, int arg)
{
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
		err_sys("fcntl error");
	return(n);
}

pid_t
Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1)
		err_sys("fork error");
	return(pid);
}

void
Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getpeername(fd, sa, salenptr) < 0)
		err_sys("getpeername error");
}

void
Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getsockname(fd, sa, salenptr) < 0)
		err_sys("getsockname error");
}

void
Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
	if (getsockopt(fd, level, optname, optval, optlenptr) < 0)
		err_sys("getsockopt error");
}

/* include Listen */
void
Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (    listen(fd, backlog) < 0)
		err_sys("listen error");
}
/* end Listen */

#ifdef	HAVE_POLL
int
Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
{
	int		n;

	if ( (n = poll(fdarray, nfds, timeout)) < 0)
		err_sys("poll error");

	return(n);
}
#endif

ssize_t
Recv(int fd, void *ptr, size_t nbytes, int flags)
{
	ssize_t		n;

	if ( (n = recv(fd, ptr, nbytes, flags)) < 0)
		err_sys("recv error");
	return(n);
}

ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr)
{
	ssize_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
		err_sys("recvfrom error");
	return(n);
}

ssize_t
Recvmsg(int fd, struct msghdr *msg, int flags)
{
	ssize_t		n;

	if ( (n = recvmsg(fd, msg, flags)) < 0)
		err_sys("recvmsg error");
	return(n);
}

int
Sockatmark(int fd)
{
	int		n;

	if ( (n = sockatmark(fd)) < 0)
		err_sys("sockatmark error");
	return(n);
}

void
Socketpair(int family, int type, int protocol, int *fd)
{
	int		n;

	if ( (n = socketpair(family, type, protocol, fd)) < 0)
		err_sys("socketpair error");
}

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

void
str_cli( FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	int		n;

	stdineof = 0;
	FD_ZERO( &rset);
	for ( ; ; ) {
		if ( stdineof == 0)
			FD_SET( fileno( fp), &rset);    // subscribe to info for fp
		FD_SET( sockfd, &rset);                 // subscribe to info for socket
		maxfdp1 = max( fileno( fp), sockfd) + 1;
                
                sigset_t newmask, oldmask, zeromask;
                sigemptyset( &zeromask);
                sigemptyset( &newmask);
                sigaddset( &newmask, SIGINT);
                sigprocmask( SIG_BLOCK, &newmask, &oldmask); /* block SIGINT */

		pselect( maxfdp1, &rset, NULL, NULL, NULL, &zeromask);

		if ( FD_ISSET( sockfd, &rset)) {	/* socket is readable */
			if ( ( n = Read( sockfd, buf, MAXLINE)) == 0) {
				if ( stdineof == 1)
					return;		/* normal termination */
				else
					err_quit( "str_cli: server terminated prematurely");
			}

			Write( fileno( stdout), buf, n);
		}

		if ( FD_ISSET( fileno( fp), &rset)) {  /* input is readable */
			if ( ( n = Read( fileno( fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				Shutdown( sockfd, SHUT_WR);	/* send FIN */
				FD_CLR( fileno(fp), &rset);    // unsubscribe from info for fp
				continue;
			}

			Writen( sockfd, buf, n);
		}
	}
}

void
str_echo(int sockfd)
{
	long		arg1, arg2;
	ssize_t		n;
	char		line[MAXLINE];

	for ( ; ; ) {
		if ( ( n = Read(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

		if ( sscanf( line, "%ld%ld", &arg1, &arg2) == 2)
			snprintf( line, sizeof(line), "%ld\n", arg1 + arg2);
		else
			snprintf( line, sizeof(line), "input error, I prefer "
                                "to receive two long numbers so I can return a sum of it\n");

		n = strlen( line);
		Writen( sockfd, line, n);
	}
}

void
sigInt( int signo)
{
    fprintf( stdout, "sig int catched");
    fflush( stdout);
    exit( 0);
}
/*
 * 
 */
int
main(int argc, char **argv)
{
        Signal( SIGINT, sigInt);
	int			i, maxi, maxfd, listenfd, connfd, sockfd;
	int			nready, client[FD_SETSIZE];
	ssize_t			n;
	fd_set			rset, allset;
	char			buf[MAXLINE];
	socklen_t		clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket( AF_INET, SOCK_STREAM, 0);

	bzero( &servaddr, sizeof( servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

        /* SO_REUSEADDR allows a new server to be started
         * on the same port as an existing server that is
         * bound to the wildcard address, as long as each
         * instance binds a different local IP address.
         * This is common for a site hosting multiple HTTP
         * servers using the IP alias technique */
        int reuseaddr_on = 1;
        if( setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR,
                &reuseaddr_on, sizeof( reuseaddr_on)) < 0)
        {
            // log
        }

	Bind( listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen( listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for ( i = 0; i < FD_SETSIZE; ++i)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO( &allset);
	FD_SET( listenfd, &allset);

	for ( ; ; ) {
		rset = allset;                  /* structure assignment */
                
                sigset_t newmask, oldmask, zeromask;
                sigemptyset( &zeromask);
                sigemptyset( &newmask);
                sigaddset( &newmask, SIGINT);
                sigprocmask( SIG_BLOCK, &newmask, &oldmask); /* block SIGINT */

		nready = pselect( maxfd + 1, &rset, NULL, NULL, NULL, &zeromask);

		if ( FD_ISSET( listenfd, &rset)) {	/* new client connection */

                    /* If the listening socket is readable, a new connection has been 
                     * established. We call accept and update our data structures
                     * accordingly. We use the first unused entry in the client array
                     * to record the connected socket. The number of ready descriptors
                     * is decremented, and if it is 0, we can avoid the next for loop.
                     * This lets us use the return value from select to avoid checking
                     * descriptors that are not ready.*/
			clilen = sizeof( cliaddr);
			connfd = Accept( listenfd, ( SA *) &cliaddr, &clilen);

			for ( i = 0; i < FD_SETSIZE; ++i)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if ( i == FD_SETSIZE)
				err_quit( "too many clients");

			FD_SET( connfd, &allset);	/* add new descriptor to set */
			if ( connfd > maxfd)
				maxfd = connfd;		/* for select */
			if ( i > maxi)
				maxi = i;		/* max index in client[] array */

			if ( --nready <= 0)
				continue;		/* no more readable descriptors */
		}

		for ( i = 0; i <= maxi; ++i) {	/* check all clients for data */
                    
                    /* A test is made for each existing client connection as to whether
                     * or not its descriptor is in the descriptor set returned by select.
                     * If so, a line is read from the client and echoed back to the client.
                     * If the client closes the connection, read returns 0 and we update
                     * our data structures accordingly. We never decrement the value
                     * of maxi, but we could check for this possibility each time a client
                     * closes its connection.*/
			if ( ( sockfd = client[i]) < 0)
				continue;
			if ( FD_ISSET( sockfd, &rset)) {
				if ( ( n = Read( sockfd, buf, MAXLINE)) == 0) {
					/* connection closed by client */
					Close( sockfd);
					FD_CLR( sockfd, &allset);
					client[i] = -1;
				} else
					Writen( sockfd, buf, n); /* echo */

				if ( --nready <= 0)
					break;		/* no more readable descriptors */
			}
		}
	}

    return 0;
}


