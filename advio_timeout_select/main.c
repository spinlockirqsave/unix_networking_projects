/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 11, 2014, 10:58 PM
 * 
 * Put timeout on recvfrom using select
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"


/*
 * 
 */
static void	sig_alrm(int);

void
dg_cli( FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while ( Fgets( sendline, MAXLINE, fp) != NULL) {

		Sendto( sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

                /* wait for reply (test socket for readability) up to 5 seconds*/
		if ( Readable_timeo( sockfd, 5) == 0) {
			fprintf( stderr, "socket timeout\n");
		} else {
			n = Recvfrom( sockfd, recvline, MAXLINE, 0, NULL, NULL);
			recvline[n] = 0;	/* null terminate */
			Fputs( recvline, stdout);
		}
	}
}

static void
sig_alrm( int signo)
{
	return;			/* not used */
}

static void
sig_chld( int signo)
{
    return;                     /* not used */
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_in	servaddr;

	if ( argc != 2)
		err_quit( "usage: udpcli <IPaddress>");

	bzero( &servaddr, sizeof( servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons( SERV_PORT);
	Inet_pton( AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	dg_cli( stdin, sockfd, (SA *) &servaddr, sizeof( servaddr));

	exit(0);
}