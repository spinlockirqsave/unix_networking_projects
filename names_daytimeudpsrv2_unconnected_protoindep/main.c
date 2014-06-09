/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 9, 2014, 4:56 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // avoid SIGSEGV as a result of incorrectly defined time_t

#include "networking_functions.h"

int
udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ( ( n = getaddrinfo( host, serv, &hints, &res)) != 0)
		err_quit( "udp_server error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol);
		if ( sockfd < 0)
			continue;		/* error - try next one */

		if ( bind( sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		Close(sockfd);		/* bind error - close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if ( res == NULL)	/* errno from final socket() or bind() */
		err_sys("udp_server error for %s, %s", host, serv);

	if ( addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(sockfd);
}
/* end udp_server */

int
Udp_server( const char *host, const char *serv, socklen_t *addrlenp)
{
	return( udp_server( host, serv, addrlenp));
}

/* Creates connected udp socket */
int
udp_connect( const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

        /* We set the address family to AF_UNSPEC, but the caller can use
         * specific address to force a particular protocol (IPv4 or IPv6)
         * i.e. 0.0.0.0 for IPv4 or 0::0 for IPv6
         */
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ( ( n = getaddrinfo( host, serv, &hints, &res)) != 0)
		err_quit( "udp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol);
		if ( sockfd < 0)
			continue;	/* ignore this one */

                /* 
                 * The call to connect with a UDP socket does not send anything to the peer.
                 * If something is wrong ( the peer is unreachable or there is no server
                 * at the specified port), the caller does not discover that until it sends
                 * a datagram to the peer.
                 */
		if ( connect( sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		Close( sockfd);	/* ignore this one */
	} while ( ( res = res->ai_next) != NULL);

	if ( res == NULL)	/* errno set from final connect() */
		err_sys( "udp_connect error for %s, %s", host, serv);

	freeaddrinfo( ressave);

	return( sockfd);
}

int
Udp_connect( const char *host, const char *serv)
{
	int		n;

	if ( ( n = udp_connect(host, serv)) < 0) {
		err_quit("udp_connect error for %s, %s: %s",
					 host, serv, gai_strerror(-n));
	}
	return(n);
}


/* Creates unconnected udp socket*/
int
udp_client( const char *host, const char *serv, SA **saptr, socklen_t *lenp)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

        /* We set the address family to AF_UNSPEC, but the caller can use
         * specific address to force a particular protocol (IPv4 or IPv6)
         * i.e. 0.0.0.0 for IPv4 or 0::0 for IPv6
         */
	bzero( &hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ( ( n = getaddrinfo( host, serv, &hints, &res)) != 0)
		err_quit( "udp_client error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol);
		if ( sockfd >= 0)
			break;		/* success */
	} while ( ( res = res->ai_next) != NULL);

	if ( res == NULL)	/* errno set from final socket() */
		err_sys( "udp_client error for %s, %s", host, serv);

	*saptr = Malloc( res->ai_addrlen);
	memcpy( *saptr, res->ai_addr, res->ai_addrlen);
	*lenp = res->ai_addrlen;

	freeaddrinfo( ressave);

	return( sockfd);
}

int
Udp_client( const char *host, const char *serv, SA **saptr, socklen_t *lenptr)
{
	return( udp_client( host, serv, saptr, lenptr));
}


int
tcp_listen( const char *host, const char *serv, socklen_t *addrlenp)
{
	int			listenfd, n;
	const int		on = 1;
	struct addrinfo         hints, *res, *ressave;

        /* We set the address family to AF_UNSPEC, but the caller can use
         * specific address to force a particular protocol (IPv4 or IPv6)
         * i.e. 0.0.0.0 for IPv4 or 0::0 for IPv6
         */
	bzero( &hints, sizeof( struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( ( n = getaddrinfo( host, serv, &hints, &res)) != 0)
		err_quit( "tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol);
		if ( listenfd < 0)
			continue;		/* error, try next one */

		Setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if ( bind( listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		Close( listenfd);	/* bind error, close and try next one */
	} while ( ( res = res->ai_next) != NULL);

	if ( res == NULL)	/* errno from final socket() or bind() */
		err_sys( "tcp_listen error for %s, %s", host, serv);

	Listen( listenfd, LISTENQ);

	if ( addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo( ressave);

	return( listenfd);
}

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */
int
Tcp_listen( const char *host, const char *serv, socklen_t *addrlenp)
{
	return( tcp_listen( host, serv, addrlenp));
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int			sockfd;
	ssize_t			n;
	char			buff[MAXLINE];
	time_t			ticks;
	socklen_t		len;
	struct sockaddr_storage	cliaddr;

	if ( argc == 2)
		sockfd = Udp_server( NULL, argv[1], NULL);
	else if ( argc == 3)
		sockfd = Udp_server( argv[1], argv[2], NULL);
	else
		err_quit("usage: daytimeudpsrv [ <host> ] <service or port>");

	for ( ; ; ) {
            
		len = sizeof(cliaddr);
                /* since socket is unconnected we use Recvfrom and Sendto,
                 * not Read, Write */
		n = Recvfrom(sockfd, buff, MAXLINE, 0, (SA *)&cliaddr, &len);
		printf("datagram from %s\n", Sock_ntop((SA *)&cliaddr, len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Sendto(sockfd, buff, strlen(buff), 0, (SA *)&cliaddr, len);
	}
}

