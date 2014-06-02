/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 2, 2014, 3:57 PM
 */

#include "networking_functions.h"
#include <time.h>

/**
 * Resolve host names.
 * Return 0 on success or EAI_* errcode to be used with gai_strerror().
 */
static int resolveHost( const char *host, unsigned int port, int family,
	struct addrinfo **res )
{
	struct addrinfo hints;

	memset( &hints, 0, sizeof(struct addrinfo));
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
	snprintf( strport, sizeof strport, "%u", port);

	int ret = getaddrinfo( host, strport, &hints, res);
        
	return ret;
}

int
Tcp_connect(int fd, const struct sockaddr *servaddr, socklen_t addrlen) {
    if ( connect( fd, servaddr, addrlen) < 0)
        err_sys( "connect error");
}

static void
usage(const char *msg);




enum { WAIT_READ = 1, WAIT_WRITE = 2 };

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
		//assert( false );
		ret = 0; // timeout
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
        if (errno == 0) {
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
    int optval = 0;
    socklen_t optlen = sizeof optval;

    if ( getsockopt( fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1) {
        
        return -1;
    }
    
    if ( optval != 0) {
        
        errno = optval;
        return -1;
    }
    
    return 0;
}

static inline void
Set_socket_nonblock( int sockfd)
{
	int flags = fcntl( sockfd, F_GETFL, 0);
        
	if( flags == -1 ) {
		err_sys( "set_socket_nonblock error");
	}
        
	if( fcntl( sockfd, F_SETFL, flags | O_NONBLOCK)  == -1 ) {
		err_sys( "set_socket_nonblock error");
	}
}

int connect_nonblocking_socket( const char* host, unsigned int port, int family) {
    
    int sockfd, m_fd, n;

    // starting to connect to server
    struct addrinfo *aiFirst;

    if ( ( n = resolveHost( host, port, family, &aiFirst)) != 0)
                err_quit( "connect error for host %s, port %d: err: %s",
                        host, port, gai_strerror (n));
    
    int con_errno = 0;
    struct addrinfo *ai;
    
    for ( ai = aiFirst; ai != NULL; ai = ai->ai_next) {

        /* try to create socket */
        m_fd = socket( ai->ai_family, ai->ai_socktype, 0);
        
        if (m_fd < 0) {
            
            con_errno = errno;
            continue;
        }

        /* set socket to O_NONBLOCK */
        Set_socket_nonblock( m_fd);

        /* try to connect with given association */
        if ( timeout_connect( m_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            
            con_errno = errno;
            Close( m_fd);
            m_fd = -1;
            /* try another association */
            continue;
        }
        /* successfully  connected */
        break;
    }
    
    freeaddrinfo( aiFirst);
    return 0;

}

static void
usage( const char *msg);

/*
 * 
 */
int main( int argc, char** argv) {

    int sockfd;
    
    if (argc < 3)
	usage("");
    
    int n = connect_nonblocking_socket( argv[1], atoi( argv[2]), AF_INET);
    if( n < 0) exit(-1);
    
    printf( "OK\n");
    sleep( 15);
    Close( sockfd);
        
    return (EXIT_SUCCESS);
}

static void
usage( const char *msg)
{
	printf( "usage: tcp_connect <host> <port>\n");

	if ( msg[0] != 0)
		printf( "%s\n", msg);
	exit(1);
}

