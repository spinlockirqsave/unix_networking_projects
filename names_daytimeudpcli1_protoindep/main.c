/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 8, 2014, 3:37 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"

/* unconnected */
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
	int			sockfd, n;
	char			recvline[MAXLINE + 1];
	socklen_t		salen;
	struct sockaddr         *sa;

	if ( argc != 3)
		err_quit( "usage: daytimeudpcli1 <hostname/IPaddress> <service/port#>");

        /* We do not set the SO_REUSEADDR socket option for the UDP socket because
         * this socket option can allow multiple sockets to bind the same UDP port
         * on hosts that support multicasting. Since there is nothing like TCP's
         * TIME_WAIT state for a UDP socket, there is no need to set this socket
         * option when the server is started.*/
	sockfd = Udp_client( argv[1], argv[2], (void **) &sa, &salen);
        
        /* At this moment we don't know if peer is reachable or not
         * If it is not reachable sendto return Success but Recvfrom
         * will block forever (it doesn't see an ICMP "destination
         * unreachable" packet coming back from peer because socket
         * is unconnected)
         */
	printf( "sending to %s\n", Sock_ntop_host(sa, salen));

	Sendto( sockfd, "", 1, 0, sa, salen);	/* send 1-byte datagram */

	n = Recvfrom( sockfd, recvline, MAXLINE, 0, NULL, NULL);
	recvline[n] = '\0';	/* null terminate */
	Fputs( recvline, stdout);

	exit(0);
}

