/* 
 * File:   main.c
 * Author: root
 *
 * Created on June 22, 2014, 5:28 PM
 */

#include <stdio.h>
#include <pthread.h>

#include "unpthreads.h"

#define	NLOOP 500

int				counter;		/* incremented by threads */

void	*doit( void *);

int
main( int argc, char **argv)
{
	pthread_t	tidA, tidB, tidC, tidD;

	Pthread_create( &tidA, NULL, &doit, NULL);
	Pthread_create( &tidB, NULL, &doit, NULL);
        Pthread_create( &tidC, NULL, &doit, NULL);
        Pthread_create( &tidD, NULL, &doit, NULL);

	/* wait for both threads to terminate */
	Pthread_join( tidA, NULL);
	Pthread_join( tidB, NULL);
        Pthread_join( tidC, NULL);
        Pthread_join( tidD, NULL);
        
        printf( "counter: %d\n", counter);

	exit(0);
}

void *
doit( void *vptr)
{
	int		i, val;

	/*
	 * Each thread fetches, prints, and increments the counter NLOOP times.
	 * The value of the counter should increase monotonically (if it was be correct).
	 */

	for ( i = 0; i < NLOOP; i++) {
            
		val = counter;
		printf( "%lu: %d\n", pthread_self(), val + 1);
		counter = val + 1;
                fflush( stdout);
	}

	return(NULL);
}

