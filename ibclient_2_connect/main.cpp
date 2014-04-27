/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on April 26, 2014, 5:02 PM
 */

extern "C" {
    #include "../../../include/unix_networking/unpv13e/lib/unp.h"
}

#include <time.h>
#include <assert.h>

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

int
main( int argc, char **argv) {
    int sockfd, n;
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
    for (struct addrinfo *ai = aitop; ai != NULL; ai = ai->ai_next) {

        // create socket
        sockfd = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sockfd < 0) {
            con_errno = errno;
            fprintf(stderr, "socket creation error\n");
            continue;
        }

        /* starting three way handshake*/
        fprintf(stderr, "starting three way handshake\n");
        if ( connect( sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
            if (errno == ETIMEDOUT) {
                fprintf(stderr, "connection timed out, no response to SYN segment\n");
            }
            if (errno == ECONNREFUSED) {
                fprintf(stderr, "no process listening on port %d\n", port);
            }
            if (errno == EHOSTUNREACH || errno == ENETUNREACH) {
                fprintf(stderr, "can't reach the server\n", port);
            }
            err_sys("connect error");
            continue;
        }

        /* successfully  created */
        break;
    }
        

//    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
//
//    bzero( &servaddr, sizeof (servaddr));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_port = htons( port); /* test */
//    
//    if ( inet_pton( AF_INET, argv[1], &servaddr.sin_addr) <= 0)
//        err_quit("inet_pton error for %s", argv[1]);
//
//    /* starting three way handshake*/
//    fprintf( stderr, "starting three way handshake\n");
//    if( connect( sockfd, (SA *) & servaddr, sizeof (servaddr)) < 0) {
//        if( errno == ETIMEDOUT) {
//            fprintf( stderr, "connection timed out, no response to SYN segment\n");
//        }
//        if( errno == ECONNREFUSED) {
//            fprintf( stderr, "no process listening on port %d\n", port);
//        }
//        if( errno == EHOSTUNREACH || errno == ENETUNREACH) {
//            fprintf( stderr, "can't reach the server\n", port);
//        }
//        err_sys( "connect error");
//    }
    fprintf( stderr, "connected\n");
    
    const int CLIENT_VERSION = 60;
    int tmp = htonl( (uint32_t)CLIENT_VERSION);
    Write( sockfd, &tmp, sizeof(tmp));
    
    
        while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* null terminate */
        //printf( "%s", recvline);
        if ( fputs(recvline, stdout) == EOF)
            err_sys( "fputs error");
        /*
        Since the stream is fully-buffered by default, not line-buffered,
        it needs to be flushed periodically.  We'll flush it here for
        demonstration purposes, even though we're about to close it.
         */
        int flush_status = fflush( stdout);
        if ( flush_status != 0) {
            puts( "\nError flushing stream!");
        } else {
            puts( "\nStream flushed.");
        }
    }
    if (n < 0)
        err_sys( "read error");
    
    fprintf( stderr, "exiting\n");
    exit(0);
}

