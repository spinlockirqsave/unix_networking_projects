/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on April 27, 2014, 2:06 PM
 */

extern "C" {
    #include "../../../include/unix_networking/unpv13e/lib/unp.h"
}

#include "twsapi_defs.h" 

#include <time.h>
#include <assert.h>
#include <sstream>
#include <vector>

	typedef std::vector<char> BytesVec;

static const size_t BufferSizeHighMark = 1 * 1024 * 1024; // 1Mb

	void CleanupBuffer(BytesVec& buffer, int processed)
{
	assert( buffer.empty() || processed <= (int)buffer.size());

	if( buffer.empty())
		return;

	if( processed <= 0)
		return;

	if( (size_t)processed == buffer.size()) {
		if( buffer.capacity() >= BufferSizeHighMark) {
			BytesVec().swap(buffer);
		}
		else {
			buffer.clear();
		}
	}
	else {
		buffer.erase( buffer.begin(), buffer.begin() + processed);
	}
};


	BytesVec m_inBuffer;
	BytesVec m_outBuffer;
        int sockfd;
        
/**
 * Resolve host names.
 * Return 0 on success or EAI_* errcode to be used with gai_strerror().
 */
static int resolveHost( const char *host, unsigned int port, int family,
	struct addrinfo **res )
{
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
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

	char strport[32];
	/* Convert the port number into a string. */
	snprintf(strport, sizeof strport, "%u", port);

	int s = getaddrinfo(host, strport, &hints, res);
	return s;
}


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
#if defined _WIN32
		/* exceptfds is only needed on windows to note "connection refused "*/
		fd_set exceptfds;
		FD_ZERO( &exceptfds );
		FD_SET( fd, &exceptfds );
		ret = select( fd + 1, NULL, &waitSet, &exceptfds, &tval );
#else
		ret = select( fd + 1, NULL, &waitSet, NULL, &tval );
#endif
		break;
	default:
		assert( false );
		ret = 0;
		break;
	}

	return ret;
}

static int timeout_connect( int fd, const struct sockaddr *serv_addr,
	socklen_t addrlen )
{
	if( connect( fd, serv_addr, addrlen) < 0 ) {
		if( errno != EINPROGRESS ) {
			return -1;
		}
	}
	if( wait_socket( fd, WAIT_WRITE  ) <= 0 ) {
		if( errno == 0 ) {
			errno = ETIMEDOUT;
		}
		return -1;
	}

	/* Completed or failed */
	int optval = 0;
	socklen_t optlen = sizeof(optval);
	/* casting  &optval to char* is required for win32 */
	if( getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) == -1 ) {
		return -1;
	}
	if (optval != 0) {
		errno = optval;
		return -1;
	}
	return 0;
}

static inline int
set_socket_nonblock( int sockfd)
{
	int flags = fcntl( sockfd, F_GETFL, 0 );
	if( flags == -1 ) {
		return -1;
	}
	if( fcntl( sockfd, F_SETFL, flags | O_NONBLOCK)  == -1 ) {
		return -1;
	}

	return 0;
}

int send(const char* buf, size_t sz)
{
	assert( sz > 0 );

	int nResult = ::send( sockfd, buf, sz, 0);

	if( nResult == -1 ) {

	}

	return nResult;
}

int sendBufferedData()
{
	if( m_outBuffer.empty())
		return 0;

	int nResult = send( &m_outBuffer[0], m_outBuffer.size());
	if( nResult <= 0) {
		return nResult;
	}
	CleanupBuffer( m_outBuffer, nResult);
	return nResult;
}

int bufferedSend(const char* buf, size_t sz)
{
	if( sz <= 0)
		return 0;

	if( !m_outBuffer.empty()) {
		m_outBuffer.insert( m_outBuffer.end(), buf, buf + sz);
		return sendBufferedData();
	}

	int nResult = send(buf, sz);

	if( nResult < (int)sz) {
		int sent = (std::max)( nResult, 0);
		m_outBuffer.insert( m_outBuffer.end(), buf + sent, buf + sz);
	}

	return nResult;
}

int bufferedSend(const std::string& msg)
{
	return bufferedSend( msg.data(), msg.size());
}

int
main( int argc, char **argv) {
    int n;
    int rc = 0;
    char buff[ MAXLINE + 1];
    char recvline[ MAXLINE + 1];
    struct sockaddr_in servaddr;
    unsigned int port = 7496;

    if ( argc != 2)
        err_quit("usage: a.out <IPaddress>");
    
    const char* host = argv[1];
    struct addrinfo *aitop;
    int s = resolveHost( host, port, AF_INET, &aitop );
    if( s != 0 ) {
		const char *err;
#ifdef HAVE_GETADDRINFO
		err = gai_strerror(s);
#else
		err = "Invalid address, hostname resolving not supported.";
#endif
		return -1;
    }

    int con_errno = 0;
    for( struct addrinfo *ai = aitop; ai != NULL; ai = ai->ai_next ) {

		// create socket
		sockfd = socket(ai->ai_family, ai->ai_socktype, 0);
		if( sockfd < 0) {
			con_errno = errno;
			continue;
		}

		/* Set socket O_NONBLOCK. If wanted we could handle errors
		   (portability!) We could even make O_NONBLOCK optional. */
		int sn = set_socket_nonblock( sockfd );
		assert( sn == 0 );

		// try to connect
		if( timeout_connect( sockfd, ai->ai_addr, ai->ai_addrlen ) < 0 ) {
			con_errno = errno;
			Close( sockfd);
			sockfd = -1;
			continue;
		}
		/* successfully  connected */
		break;
	}

	freeaddrinfo(aitop);

    fprintf( stderr, "connected\n");
    
    const int CLIENT_VERSION = 60;
    int tmp = htonl( (uint32_t)CLIENT_VERSION);
    std::ostringstream oss;
    oss << tmp << '\0';
    bufferedSend( oss.str());
    //Write( sockfd, oss.str().data(), sizeof(tmp));
    
    int u = 300000;
        while ( u--) {

    }
    if (n < 0)
        err_sys( "read error");
    
    fprintf( stderr, "exiting\n");
    exit(0);
}



