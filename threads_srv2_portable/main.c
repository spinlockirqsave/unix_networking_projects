/* 
 * File:   main.c
 * Author: piter
 *
 * Created on June 20, 2014, 10:59 PM
 * 
 * Each time we call accept, we first call malloc
 * and allocate space for an integer variable,
 * the connected descriptor. This gives each thread
 * its own copy of the connected descriptor.
 * The thread fetches the value of the connected
 * descriptor and then calls free to release the memory.
 * 
 * Historically, the malloc and free functions have been nonre-entrant.
 * That is, calling either function from a signal handler while the main
 * thread is in the middle of one of these two functions has been a recipe
 * for disaster, because of static data structures that are manipulated
 * by these two functions. How can we call these two functions here?
 * POSIX requires that these two functions, along with many others,
 * be thread-safe. This is normally done by some form of synchronization
 * performed within the library functions that is transparent to us.
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
		err_quit( "usage: threads_srv2 [ <host> ] <service or port>");

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
	Pthread_detach( tid);           /* so we don't join it in main thread */
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