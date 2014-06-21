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




static void	*doit(void *);		/* each thread executes this function */
typedef pthread_t thread_t;

/*
 * 
 */
int
main( int argc, char **argv)
{
	int			listenfd, *iptr;
	thread_t		tid;
	socklen_t		addrlen, len;
	struct sockaddr	*cliaddr;

	if ( argc == 2)
		listenfd = Tcp_listen( NULL, argv[1], &addrlen);
	else if ( argc == 3)
		listenfd = Tcp_listen( argv[1], argv[2], &addrlen);
	else
		err_quit("usage: threads_srv2 [ <host> ] <service or port>");

	cliaddr = Malloc(addrlen);

	for ( ; ; ) {
            
            /* 
             * we allocate storage for connection socket on heap
             * and we pass a pointer to it as last argument to
             * pthread_create ( as a generic void* of course)
             */
		len = addrlen;
		iptr = Malloc(sizeof(int));
		*iptr = Accept( listenfd, cliaddr, &len);
		Pthread_create( &tid, NULL, &doit, iptr);
	}
}

void str_echo( int);

static void *
doit(void *arg)
{
	int		connfd;
        pthread_t       tid;

	connfd = *((int *) arg);
	free(arg);

        tid = pthread_self();
	Pthread_detach( tid);
	str_echo(connfd);		/* same function as before */
	Close(connfd);			/* done with connected socket */
	return(NULL);
}

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