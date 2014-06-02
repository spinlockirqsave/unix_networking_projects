/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 2, 2014, 1:48 PM
 */

#include "networking_functions.h"

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

	char strport[32];
	/* Convert the port number into a string. */
	snprintf( strport, sizeof strport, "%u", port);

	int ret = getaddrinfo( host, strport, &hints, res);
        
	return ret;
}

int tcp_connect( const char *host, unsigned int port, int family) {
    
	int				sockfd, n;
	struct  addrinfo                hints, *res, *ressave;

	bzero( &hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
        
        /* Convert the port number into a string. */
        char strport[32];
	snprintf( strport, sizeof strport, "%u", port);

	if ( ( n = getaddrinfo( host, strport, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s",
				 host, strport, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol);
		if ( sockfd < 0)
			continue;	/* ignore this one */

		if ( connect( sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		Close(sockfd);	/* ignore this one */
	} while ( ( res = res->ai_next) != NULL);

	if ( res == NULL)	/* errno set from final connect() */
		err_sys( "tcp_connect error for %s, %s", host, strport);

	freeaddrinfo( ressave);

	return( sockfd);
}

int
Tcp_connect( const char *host, unsigned int port, int family)
{
	return( tcp_connect( host, port, family));
}

static void
usage(const char *msg);

/*
 * 
 */
int main(int argc, char** argv) {

    int sockfd;
    
    if (argc < 3)
	usage("");
    
    sockfd = Tcp_connect( argv[1], atoi( argv[2]), AF_INET);
    if( sockfd < 0) exit(-1);
    
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

