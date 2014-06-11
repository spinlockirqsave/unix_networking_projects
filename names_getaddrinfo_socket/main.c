/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 1, 2014, 3:18 PM
 */

#include <stdio.h>
#include <stdlib.h>


/*
 * Test program for getaddrinfo() and getnameinfo().
 */

/* globals */
int		vflag;
#define IPv4
#define IPv6

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

/* function prototypes for internal functions */
static void	do_errtest(void);
static void	do_funccall(const char *, const char *, int, int, int, int, int);
static int	do_onetest(char *, char *, struct addrinfo *, int);
static const char *str_fam(int);
static const char *str_sock(int);
static void	usage(const char *);

/* POSIX requires that an #include of <poll.h> DefinE INFTIM, but many
   systems still DefinE it in <sys/stropts.h>.  We don't want to include
   all the STREAMS stuff if it's not needed, so we just DefinE INFTIM here.
   This is the standard value, but there's no guarantee it is -1. */

#define INFTIM          (-1)    /* infinite poll timeout */
#define	HAVE_POLL

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

void
err_sys(const char *fmt, ...);

long
open_max(void)
{
    if ( openmax == 0) {     /* first time through */
        errno = 0;
        if (( openmax = sysconf( _SC_OPEN_MAX)) < 0) {
            if ( errno == 0)
                openmax = OPEN_MAX_GUESS;   /* it's indeterminate */
            else
                err_sys( "sysconf error for _SC_open_max()");
        }
    }

    return(openmax);
}

#define MAXLINE 4096
#define SERV_PORT 9877
#define SA struct sockaddr
int		daemon_proc;		/* set nonzero by daemon_init() */

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#define	LISTENQ		1024	/* 2nd argument to listen() */

typedef void (Sigfunc)(int);    /* signal handler */
static void	err_doit(int, int, const char *, va_list);

/* Nonfatal error related to system call
 * Print message and return */
void
err_ret(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_INFO, fmt, ap);
	va_end(ap);
	return;
}

/* Nonfatal error unrelated to system call
 * Print message and return */
void
err_msg(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, LOG_INFO, fmt, ap);
	va_end(ap);
	return;
}

/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */
static void
err_doit( int errnoflag, int level, const char *fmt, va_list ap)
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

/*
 * Return a string containing some additional information after a
 * host name or address lookup error - gethostbyname() or gethostbyaddr().
 *
 * This is only compiled if the local host does not provide it--recent
 * versions of BIND supply this function.
 */
const char *
hstrerror(int err)
{
	if (err == 0)
		return("no error");

	if (err == HOST_NOT_FOUND)
		return("Unknown host");

	if (err == TRY_AGAIN)
		return("Hostname lookup failure");

	if (err == NO_RECOVERY)
		return("Unknown server error");

	if (err == NO_DATA)
        return("No address associated with name");

	return("unknown error");
}

/* inet_ntop protocol independent */
#include <sys/un.h>

char *
sock_ntop( const struct sockaddr *sa, socklen_t salen)
{
    char		portstr[8];
    static char         str[128];		/* Unix domain is largest */

	switch ( sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if ( inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		str[0] = '[';
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return(str);
		}
		return (str + 1);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s (index %d)",
					 sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
	char	*ptr;

	if ( ( ptr = sock_ntop(sa, salen)) == NULL)
		err_sys("sock_ntop error");	/* inet_ntop() sets errno */
	return(ptr);
}

const char *
Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const char	*ptr;

	if (strptr == NULL)		/* check for old code */
		err_quit( "NULL 3rd argument to inet_ntop");
	if ( (ptr = inet_ntop( family, addrptr, strptr, len)) == NULL)
		err_sys( "inet_ntop error");		/* sets errno */
	return(ptr);
}

void
Inet_pton(int family, const char *strptr, void *addrptr)
{
	int		n;

	if ( (n = inet_pton( family, strptr, addrptr)) < 0)
		err_sys( "inet_pton error for %s", strptr);	/* errno set */
	else if ( n == 0)
		err_quit( "inet_pton error for %s", strptr);	/* errno not set */

	/* nothing to return */
}
/*
 *
 */
int
main(int argc, char **argv)
{
	int				doerrtest = 0;
	int				loopcount = 1;
	int				c, i;
	char			*host = NULL;
	char			hostbuf[NI_MAXHOST];
	char			*serv = NULL;
	char			servbuf[NI_MAXSERV];
	struct protoent	*proto;
	struct addrinfo	hints;		/* set by command-line options */

	if (argc < 2)
		usage("");

	memset(&hints, 0, sizeof(struct addrinfo));

	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "cef:h:l:pr:s:t:v")) != -1) {
		switch (c) {
		case 'c':
			hints.ai_flags |= AI_CANONNAME;
			break;

		case 'e':
			doerrtest = 1;
			break;

		case 'f':			/* address family */
#ifdef	IPv4
			if (strcmp(optarg, "inet") == 0) {
				hints.ai_family = AF_INET;
				break;
			}
#endif
#ifdef	IPv6
			if (strcmp(optarg, "inet6") == 0) {
				hints.ai_family = AF_INET6;
				break;
			}
#endif
#ifdef	UNIXdomain
			if (strcmp(optarg, "unix") == 0) {
				hints.ai_family = AF_LOCAL;
				break;
			}
#endif
			usage("invalid -f option");

		case 'h':			/* host */
			strncpy(hostbuf, optarg, NI_MAXHOST-1);
			host = hostbuf;
			break;

		case 'l':			/* loop count */
			loopcount = atoi(optarg);
			break;

		case 'p':
			hints.ai_flags |= AI_PASSIVE;
			break;

		case 'r':			/* protocol */
			if ((proto = getprotobyname(optarg)) == NULL) {
				hints.ai_protocol = atoi(optarg);
			} else {
				hints.ai_protocol = proto->p_proto;
			}
			break;

		case 's':
			strncpy(servbuf, optarg, NI_MAXSERV-1);
			serv = servbuf;
			break;

		case 't':			/* socket type */
			if (strcmp(optarg, "stream") == 0) {
				hints.ai_socktype = SOCK_STREAM;
				break;
			}

			if (strcmp(optarg, "dgram") == 0) {
				hints.ai_socktype = SOCK_DGRAM;
				break;
			}

			if (strcmp(optarg, "raw") == 0) {
				hints.ai_socktype = SOCK_RAW;
				break;
			}

#ifdef	SOCK_RDM
			if (strcmp(optarg, "rdm") == 0) {
				hints.ai_socktype = SOCK_RDM;
				break;
			}
#endif

#ifdef	SOCK_SEQPACKET
			if (strcmp(optarg, "seqpacket") == 0) {
				hints.ai_socktype = SOCK_SEQPACKET;
				break;
			}
#endif
			usage("invalid -t option");

		case 'v':
			vflag = 1;
			break;

		case '?':
			usage("unrecognized option");
		}
	}
	if (optind < argc) {
		usage("extra args");
	}

	if (doerrtest) {
		do_errtest();
		exit(0);
	}

	for (i = 1; i <= loopcount; i++) {
		if (do_onetest(host, serv, &hints, i) > 0)
			exit(1);

		if (i % 1000 == 0) {
			printf(" %d", i);
			fflush(stdout);
		}
	}

	exit(0);
}

/*
 * Check that the right error codes are returned for invalid input.
 * Test all the errors that are easy to test for.
 */

static void
do_errtest(void)
{
		/* passive open with no hostname and no address family */
	do_funccall(NULL, "ftp", AI_PASSIVE, 0, 0, 0, 0);

		/* kind of hard to automatically test EAI_AGAIN ??? */

		/* invalid flags */
	do_funccall("localhost", NULL, 999999, 0, 0, 0, EAI_BADFLAGS);

		/* how to test EAI_FAIL ??? */

		/* invalid address family */
	do_funccall("localhost", NULL, 0, AF_SNA, 0, 0, EAI_FAMILY);

		/* hard to test for EAI_MEMORY: would have to malloc() until
		   failure, then give some back, then call getaddrinfo and
		   hope that its memory requests would not be satisfied. */

		/* to test for EAI_NODATA: would have to know of a host with
		   no A record in the DNS */

#ifdef	notdef	/* following depends on resolver, sigh */
		/* believe it or not, there is a registered domain "bar.com",
		   so the following should generate NO_DATA from the DNS */
	do_funccall("foo.bar.foo.bar.foo.bar.com", NULL, 0, 0, 0, 0, EAI_NODATA);
#endif

		/* no hostname, no service name */
	do_funccall(NULL, NULL, 0, 0, 0, 0, EAI_NONAME);

		/* invalid hostname (should be interpreted in local default domain) */
	do_funccall("lkjjkhjhghgfgfd", NULL, 0, 0, 0, 0, EAI_NONAME);

		/* invalid service name */
	do_funccall(NULL, "nosuchservice", 0, 0, 0, 0, EAI_NONAME);

		/* service valid but not supported for socket type */
	do_funccall("localhost", "telnet", 0, 0, SOCK_DGRAM, 0, EAI_SERVICE);

		/* service valid but not supported for socket type */
	do_funccall("localhost", "tftp", 0, 0, SOCK_STREAM, 0, EAI_SERVICE);

		/* invalid socket type */
	do_funccall("localhost", NULL, 0, AF_INET, SOCK_SEQPACKET, 0, EAI_SOCKTYPE);

		/* EAI_SYSTEM not generated by my implementation */
}

static void
do_funccall(const char *host, const char *serv,
			int flags, int family, int socktype, int protocol, int exprc)
{
	int				rc;
	struct addrinfo	hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = flags;
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_protocol = protocol;

	rc = getaddrinfo(host, serv, &hints, &res);
	if (rc != exprc) {
		printf("expected return = %d (%s),\nactual return = %d (%s)\n",
				exprc, gai_strerror(exprc), rc, gai_strerror(rc));
		if (host != NULL)
			printf("  host = %s\n", host);
		if (serv != NULL)
			printf("  serv = %s\n", serv);
		printf("  flags = %d, family = %s, socktype = %s, protocol = %d\n",
				flags, str_fam(family), str_sock(socktype), protocol);
		exit(2);
	}
}

static int 
do_onetest(char *host, char *serv, struct addrinfo *hints, int iteration)
{
	int				rc, fd, verbose;
	struct addrinfo	*res, *rescopy;
	char			rhost[NI_MAXHOST], rserv[NI_MAXSERV];

	verbose = vflag && (iteration == 1);	/* only first time */

	if (host != NULL && verbose)
		printf("host = %s\n", host);
	if (serv != NULL && verbose)
		printf("serv = %s\n", serv);

	rc = getaddrinfo(host, serv, hints, &res);
	if (rc != 0) {
		printf("getaddrinfo return code = %d (%s)\n", rc, gai_strerror(rc));
		return(1);
	}

	rescopy = res;
	do {
		if (iteration == 1) {	/* always print results first time */
			printf("\nsocket(%s, %s, %d)", str_fam(res->ai_family),
					str_sock(res->ai_socktype), res->ai_protocol);

				/* canonname should be set only in first addrinfo{} */
			if (hints->ai_flags & AI_CANONNAME) {
				if (res->ai_canonname)
					printf(", ai_canonname = %s", res->ai_canonname);
			}
			printf("\n");

			printf("\taddress: %s\n",
				   Sock_ntop(res->ai_addr, res->ai_addrlen));
		}

			/* Call socket() to make sure return values are valid */
		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd < 0)
			printf("call to socket() failed!\n");
		else
			close(fd);

		/*
		 * Call getnameinfo() to check the reverse mapping.
		 */

		rc = getnameinfo(res->ai_addr, res->ai_addrlen,
						 rhost, NI_MAXHOST, rserv, NI_MAXSERV,
						 (res->ai_socktype == SOCK_DGRAM) ? NI_DGRAM: 0);
		if (rc == 0) {
			if (verbose)
				printf("\tgetnameinfo: host = %s, serv = %s\n",
					   rhost, rserv);
		} else
			printf("getnameinfo returned %d (%s)\n", rc, gai_strerror(rc));

	} while ( (res = res->ai_next) != NULL);

	freeaddrinfo(rescopy);
	return(0);
}

static void
usage(const char *msg)
{
	printf(
"usage: testaddrinfo [ options ]\n"
"options: -h <host>    (can be hostname or address string)\n"
"         -s <service> (can be service name or decimal port number)\n"
"         -c    AI_CANONICAL flag\n"
"         -p    AI_PASSIVE flag\n"
"         -l N  loop N times (check for memory leaks with ps)\n"
"         -f X  address family, X = inet, inet6, unix\n"
"         -r X  protocol, X = tcp, udp, ... or number e.g. 6, 17, ...\n"
"         -t X  socket type, X = stream, dgram, raw, rdm, seqpacket\n"
"         -v    verbose\n"
"         -e    only do test of error returns (no options required)\n"
"  without -e, one or both of <host> and <service> must be specified.\n"
);

	if (msg[0] != 0)
		printf("%s\n", msg);
	exit(1);
}

static const char *
str_fam( int family)
{
#ifdef	IPv4
	if (family == AF_INET)
		return("AF_INET");
#endif
#ifdef	IPv6
	if (family == AF_INET6)
		return("AF_INET6");
#endif
#ifdef	UNIXdomain
	if (family == AF_LOCAL)
		return("AF_LOCAL");
#endif
	return("<unknown family>");
}

static const char *
str_sock(int socktype)
{
	switch(socktype) {
	case SOCK_STREAM:	return "SOCK_STREAM";
	case SOCK_DGRAM:	return "SOCK_DGRAM";
	case SOCK_RAW:		return "SOCK_RAW";
#ifdef SOCK_RDM
	case SOCK_RDM:		return "SOCK_RDM";
#endif
#ifdef SOCK_SEQPACKET
	case SOCK_SEQPACKET:return "SOCK_SEQPACKET";
#endif
	default:		return "<unknown socktype>";
	}
}

