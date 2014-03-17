#include "../../../include/unix_networking/unpv13e/lib/unp.h"

#include <time.h>
#include <assert.h>



static inline int
set_socket_nonblock(int sockfd)
{
#if defined _WIN32
	unsigned long mode = 1;
	if( ioctlsocket(sockfd, FIONBIO, &mode) != NO_ERROR ) {
		return -1;
	}
#else
	int flags = fcntl( sockfd, F_GETFL, 0 );
	if( flags == -1 ) {
		return -1;
	}
	if( fcntl(sockfd, F_SETFL, flags | O_NONBLOCK)  == -1 ) {
		return -1;
	}
#endif
	return 0;
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
		assert( 0 );
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


int
main(int argc, char **argv) {
    
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("usage: a.out <IPaddress>");

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket error");

    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( 7496); /* IB TWS */
    if ( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_quit("inet_pton error for %s", argv[1]);

    
    
//    int con_errno = 0;
//    // create socket
//		sockfd = socket(AF_INET, SOCK_STREAM, 0);
//		if( sockfd < 0) {
//			con_errno = errno;
//			//continue;
//		}
//
//		/* Set socket O_NONBLOCK. If wanted we could handle errors
//		   (portability!) We could even make O_NONBLOCK optional. */
//		int sn = set_socket_nonblock( sockfd );
//		assert( sn == 0 );
//
//		// try to connect
//		if( timeout_connect( sockfd, (SA *) & servaddr, sizeof (servaddr) ) < 0 ) {
//			con_errno = errno;
//			close( sockfd);
//			sockfd = -1;
//			//continue;
//		}
//		/* successfully  connected */
    
    

    if ( connect(sockfd, (SA *) & servaddr, sizeof (servaddr)) < 0)
        err_sys( "connect error");

    const char* buf = "53";
    const int CLIENT_VERSION    = 53;
    int nResult = send( sockfd, buf, 2, 0);
    printf( "nRes:%d",nResult);
    
    
    if ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* null terminate */
        //printf( "%s", recvline);
        if ( fputs(recvline, stdout) == EOF)
            err_sys( "fputs error");
        /*
        Since the stream is fully-buffered by default, not line-buffered,
        it needs to be flushed periodically.  We'll flush it here for
        demonstration purposes, even though we're about to close it.
         */
        int flush_status = fflush(stdout);
        if ( flush_status != 0) {
            puts("\nError flushing stream!");
        } else {
            puts("\nStream flushed.");
        }
    }
    if (n < 0)
        err_sys("read error");

    fd_set readSet, writeSet;

	struct timeval tval;
	tval.tv_usec = 0;
	tval.tv_sec = 60;

	time_t now = time(NULL);

//	switch (m_state) {
//		case ST_PLACEORDER:
//			//placeOrder_MSFT();
//                    printf("PosixClient: ST_PLACEORDER\n");
//                    //reqMktData_MSFT();
//			break;
//		case ST_PLACEORDER_ACK:
//                        printf("PosixClient: ST_PLACEORDER_ACK\n");
//                        //reqMktData_MSFT();
//			break;
//		case ST_CANCELORDER:
//                        printf("PosixClient: ST_CANCELORDER\n");
//			//cancelOrder();
//			break;
//		case ST_CANCELORDER_ACK:
//                        printf("PosixClient: ST_CANCELORDER_ACK\n");
//			break;
//                case ST_REQMKTDATA:
//                        printf("PosixClient: ST_REQMKTDATA\n");
//			//cancelOrder();
//			break;
//		case ST_REQMKTDATA_ACK:
//                        //printf("PosixClient: ST_REQMKTDATA_ACK\n");
//			break;        
//		case ST_PING:
//			printf("PosixClient: ST_PING\n");
//                        //reqCurrentTime();
//			break;
//		case ST_PING_ACK:
//                        printf("PosixClient: ST_PING_ACK\n");
//			if( m_sleepDeadline < now) {
//				disconnect();
//				return;
//			}
//			break;
//		case ST_IDLE:
//                        printf("PosixClient: ST_IDLE\n");
//			if( m_sleepDeadline < now) {
//				m_state = ST_PING;
//				return;
//			}
//			break;
//	}


	if( sockfd >= 0 ) {

		FD_ZERO( &readSet);
		writeSet = readSet;

		FD_SET( sockfd, &readSet);

		//if( !m_pClient->isOutBufferEmpty())
			//FD_SET( sockfd, &writeSet);

		int ret = select( sockfd + 1, &readSet, &writeSet, NULL, &tval);
                printf("ret:%d",ret);
		if( ret == 0) { // timeout
                        //printf("PosixClient::processMessages: timeout\n");
			return -100;
		}

		if( ret < 0) {	// error
                        printf("PosixClient::processMessages: disconnect\n");
			//disconnect();
			return - 100;
		}

		if( FD_ISSET( sockfd, &writeSet)) {
			// socket is ready for writing
                        printf("PosixClient::processMessages: onSend\n");
			//m_pClient->onSend();
		}

		if( sockfd < 0)
			return -100;

		if( FD_ISSET( sockfd, &readSet)) {
			// socket is ready for reading
                        printf("onReceive ::::::::::::::::::::::::::\n");
                        // will call EPosixClientSocket::onReceive()
                }
        }
                        
    exit(0);
}
