/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 20, 2014, 10:59 PM
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "networking_functions.h"




static void *doit( void *);		/* each thread executes this function */

void
str_echo( int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE];

again:
	while ( ( n = read( sockfd, buf, MAXLINE)) > 0)
		Writen( sockfd, buf, n);

	if ( n < 0 && errno == EINTR)
		goto again;
	else if ( n < 0)
		err_sys( "str_echo: read error");
}



/*
 * 
 */
int
main( int argc, char **argv)
{
	int		listenfd, connfd;
	pthread_t	tid;
	socklen_t	addrlen, len;
	struct sockaddr	*cliaddr;

	if ( argc == 2)
		listenfd = Tcp_listen( NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen( argv[1], argv[2], &addrlen);
	else
		err_quit( "usage: threads_srv1 [ <host> ] <service or port>");

	cliaddr = Malloc( addrlen);

	for ( ; ; ) {
		len = addrlen;
		connfd = Accept( listenfd, cliaddr, &len);
                
                /* 
                 * Pass int value as a generic pointer to void.
                 * This way the integer value of descriptor is copied
                 * so each thread operates on different descriptor,
                 * not on same connfd ( as it would be if we passed
                 * &connfd instead)
                 */
		Pthread_create( &tid, NULL, &doit, (void *) connfd);
	}
}

static void *
doit( void *arg)
{
    //printf( "tid=%d,self=%d\n", tid, pthread_self());
    pthread_t pt = pthread_self();
	Pthread_detach( pt);
	str_echo( (int) arg);	/* same function as before */
	Close( (int) arg);		/* done with connected socket */
	return(NULL);
}