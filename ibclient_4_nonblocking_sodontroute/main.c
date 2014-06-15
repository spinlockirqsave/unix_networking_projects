/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 13, 2014, 6:10 PM
 * 
 * Nonblocking socket with SO_DONTROUTE flag
 */

#include "networking_functions.h"
#include <time.h>
#include <assert.h>
#include <netinet/tcp.h>

#define IPv4
#define IPv6
#define UNIXdomain

static const char *str_fam(int);
static const char *str_sock(int);

char m_inBuffer[MAXLINE];
char m_outBuffer[MAXLINE];
int sockfd;

void CleanupBuffer( char* buffer, int processed)
{
    memset( buffer, 0, processed);
}

int sendBufferedData()
{
	if( strlen( m_outBuffer) == 0)
		return 0;

	int nResult = Send( sockfd, &m_outBuffer[0], strlen( m_outBuffer));
	if( nResult <= 0) {
		return nResult;
	}
	CleanupBuffer( m_outBuffer, nResult);
	return nResult;
}

int bufferedSend( const char* buf, size_t sz)
{
	if( sz <= 0)
		return 0;

	if( strlen( m_outBuffer) > 0) {
		strncpy( m_outBuffer, buf, sz);
		return sendBufferedData();
	}

	int nResult = Send( sockfd, buf, sz);

	if( nResult < (int)sz) {
		int sent = max( nResult, 0);
		strncpy( m_outBuffer + strlen(m_outBuffer), buf + sent, sz - sent);
	}

	return nResult;
}

/**
 * Resolve host names.
 * Return 0 on success or EAI_* errcode to be used with gai_strerror().
 */
static int resolveHost(const char *host, unsigned int port, int family,
        struct addrinfo **res) {
    
    /* specifies the type of socket address structure
     * that we expect to be returned */
    struct addrinfo hints;

    memset( &hints, 0, sizeof ( struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
#if HAVE_DECL_AI_ADDRCONFIG
    hints.ai_flags |= AI_ADDRCONFIG;
#endif
#if HAVE_DECL_AI_V4MAPPED
    hints.ai_flags |= AI_V4MAPPED;
#endif
    hints.ai_protocol = 0;

    /* Convert the port number into a string. */
    char strport[32];
    snprintf(strport, sizeof strport, "%u", port);

    /* reentrant */
    int ret = getaddrinfo( host, strport, &hints, res);
    if ( ret != 0) {
        err_quit("resolveHost error for host %s, port %d: %s",
                host, port, gai_strerror (ret));

    }

    return ret;
}

int
Tcp_connect( int fd, const struct sockaddr *servaddr, socklen_t addrlen) {
    if ( connect( fd, servaddr, addrlen) < 0)
        err_sys( "connect error");
}

static void
usage(const char *msg);


enum Wait {
    WAIT_READ = 0,
    WAIT_WRITE = 1
};

/* wrapper, jacket around the system call select*/
static int wait_socket( int fd, int flag )
{
	errno = 0;
	const int timeout_msecs = 5000;

	struct timeval tval;
	tval.tv_usec = 1000 * (timeout_msecs % 1000);
	tval.tv_sec = timeout_msecs / 1000;

	fd_set waitSet;
	FD_ZERO( &waitSet );
	FD_SET( fd, &waitSet );

	int ret;
	switch( flag ) {
	case WAIT_READ:
		ret = select( fd + 1, &waitSet, NULL, NULL, &tval );
		break;
	case WAIT_WRITE:

		ret = select( fd + 1, NULL, &waitSet, NULL, &tval );
		break;
	default:
		err_sys( "wait_socket default - never should get here");
		ret = 0;
		break;
	}

	return ret;
}

static int timeout_connect( int fd, const struct sockaddr *serv_addr, socklen_t addrlen) {
    
    int res;
    
    if ( ( res = connect( fd, serv_addr, addrlen)) < 0) {
        
        if ( errno != EINPROGRESS) {

            err_ret( "timeout_connect error, %s", strerror( errno));
            return -1;
        }
    }
    
    if( res == 0) return 0;
    
    if ( wait_socket( fd, WAIT_WRITE) <= 0) {
        
        /* if timeout */
        if ( errno == 0) {
            errno = ETIMEDOUT;
        }
        /* maybe interrupted by a caught signal */
        err_ret( "timeout_connect for wait_socket error, %s", strerror( errno));
        return -1;
    }

    /* 
     * Completed the connection or the connect has failed
     * Check the result of the connection attempt
     */
    int optval = 0; // value-result variable for pending error
    socklen_t optlen = sizeof optval;

    /* get pending error ( if any) and clear:
     * The value of so_error ( socket member)
     * is reset to 0 by the kernel
     */
    if ( getsockopt( fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1) {
        
        err_ret( "getsockopt error");
        return -1;
    }
    
    /* pending errors? */
    if ( optval != 0) {
        
        errno = optval;
        err_ret( "getsockopt fetched error, %s", strerror( errno));
        return -1;
    }
    
    return 0;
}

/* 
 * POSIX way to set a socket for nonblocking I/O
 * that is: using fcntl ( file control) function.
 * POSIX specifies that fcntl is the preferred way
 * over ioctl
 */
static inline void
Set_socket_for_nonblocking_io( int sockfd)
{
	int flags = fcntl( sockfd, F_GETFL, 0);
        
	if( flags == -1 ) {
		err_sys( "Set_socket_for_nonblocking_io fcntl F_GETFL error");
	}
        
        flags |= O_NONBLOCK;
	if( fcntl( sockfd, F_SETFL, flags)  == -1 ) {
		err_sys( "Set_socket_for_nonblocking_io fcntl F_SETFL error");
	}
}

int connect_nonblocking_socket( const char* host, unsigned int port, int family) {
    
    int m_fd, n;

    /* connect to server */
    struct addrinfo *aiFirst;

    if ( ( n = resolveHost( host, port, family, &aiFirst)) != 0)
                err_quit( "connect error for host %s, port %d: err: %s",
                        host, port, gai_strerror (n));
    
    /* Try each addrinfo structure until success or end of list*/
    int con_errno = 0;
    struct addrinfo *ai;
    
    for ( ai = aiFirst; ai != NULL; ai = ai->ai_next) {

        /* try to create socket */
        m_fd = socket( ai->ai_family, ai->ai_socktype, 0);
        
        if ( m_fd < 0) {
            
            con_errno = errno;
            continue;
        }

        /* set socket to O_NONBLOCK */
        Set_socket_for_nonblocking_io( m_fd);

        /* try to connect with given association */
        if ( timeout_connect( m_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            
            con_errno = errno;
            Close( m_fd);
            m_fd = -1;
            /* try another association */
            continue;
        }
        
        /* successfully  connected */
        err_ret( "Success: host %s, port %d, family %s, socktype %s",
                host, port, str_fam( ai->ai_family), str_sock( ai->ai_socktype));
        break;
    }
    
    if( ai == NULL) {

        err_ret( "connect_nonblocking_socket failure");
        return -1;
    }
    freeaddrinfo( aiFirst);
    return m_fd;

}

static void
usage( const char *msg);

/*
 * 
 */
int main( int argc, char** argv) {
    
    if (argc < 3)
	usage("");
    
    sockfd = connect_nonblocking_socket( argv[1], atoi( argv[2]), AF_INET);
    if( sockfd < 0) exit(-1);
    
    /* Bypass routing table lookup */
    int option = 1;
    Setsockopt( sockfd, SOL_SOCKET, SO_DONTROUTE, &option, sizeof(option));
    
    /* Disable Nagle's algorithm */
    option = 1;
    Setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option));
    
    printf( "OK\n");
    sleep( 1);
    
    
    if ( wait_socket( sockfd, WAIT_WRITE) <= 0) {
        
        /* if timeout */
        if ( errno == 0) {
            errno = ETIMEDOUT;
        }
        /* maybe interrupted by a caught signal */
        err_ret( "main, wait_socket error, %s", strerror( errno));
        return -1;
    }
    printf( "OK. Ready to write. strlen(m_outBuffer)=%d\n", strlen(m_outBuffer));
    
    const int CLIENT_VERSION = 60;
    int tmp = htonl( (uint32_t)CLIENT_VERSION);
    char buf[10];
    strcpy( buf, "60");
    bufferedSend( buf, 3);
    printf( "OK. CLIENT_VERSION sent\n");
    
    if ( wait_socket( sockfd, WAIT_READ) <= 0) {
        
        /* if timeout */
        if ( errno == 0) {
            errno = ETIMEDOUT;
        }
        /* maybe interrupted by a caught signal */
        err_ret( "main, wait_socket error, %s", strerror( errno));
        return -1;
    }
    
    printf( "OK. Ready to read\n");
    int n = Read( sockfd, buf, MAXLINE);
    printf( "Read:%s\n", buf);
    
    
    Close( sockfd);
        
    return (EXIT_SUCCESS);
}

static void
usage( const char *msg)
{
	printf( "usage: tcp_connect <host> <port>\n"
                "\n"
                "example:\n"
                "./ibclient_4 localhost 7496\n"
        );

	if ( msg[0] != 0)
		printf( "%s\n", msg);
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
str_sock( int socktype)
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