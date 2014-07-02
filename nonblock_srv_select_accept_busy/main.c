/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on July 3, 2014, 12:57 AM
 *
 * This server avoids all the overhead of creating a new process
 * for each client and it is a nice example of select. Nevertheless,
 * there is a problem with this server that can be easily fixed
 * by making the listening socket nonblocking and then checking
 * for, and ignoring, a few errors from accept
 * ( deliberately not fixed here as an example)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"



void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( ( pid = waitpid( -1, &stat, WNOHANG)) > 0) {
		printf( "child %d terminated\n", pid);
	}
	return;
}

void
str_cli( FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	int		n;

	stdineof = 0;
	FD_ZERO( &rset);
	for ( ; ; ) {
		if ( stdineof == 0)
			FD_SET( fileno( fp), &rset);    // subscribe to info for fp
		FD_SET( sockfd, &rset);                 // subscribe to info for socket
		maxfdp1 = max( fileno( fp), sockfd) + 1;
		Select( maxfdp1, &rset, NULL, NULL, NULL);

		if ( FD_ISSET( sockfd, &rset)) {	/* socket is readable */
			if ( ( n = Read( sockfd, buf, MAXLINE)) == 0) {
				if ( stdineof == 1)
					return;		/* normal termination */
				else
					err_quit( "str_cli: server terminated prematurely");
			}

			Write( fileno( stdout), buf, n);
		}

		if ( FD_ISSET( fileno( fp), &rset)) {  /* input is readable */
			if ( ( n = Read( fileno( fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				Shutdown( sockfd, SHUT_WR);	/* send FIN */
				FD_CLR( fileno(fp), &rset);    // unsubscribe from info for fp
				continue;
			}

			Writen( sockfd, buf, n);
		}
	}
}

void
str_echo(int sockfd)
{
	long		arg1, arg2;
	ssize_t		n;
	char		line[MAXLINE];

	for ( ; ; ) {
		if ( ( n = Read(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

		if ( sscanf( line, "%ld%ld", &arg1, &arg2) == 2)
			snprintf( line, sizeof(line), "%ld\n", arg1 + arg2);
		else
			snprintf( line, sizeof(line), "input error, I prefer "
                                "to receive two long numbers so I can return a sum of it\n");

		n = strlen( line);
		Writen( sockfd, line, n);
	}
}

void
sigInt( int signo)
{
    fprintf( stdout, "sig int catched");
    fflush( stdout);
    exit( 0);
}

/*
 * 
 */
int
main(int argc, char **argv)
{
        Signal( SIGINT, sigInt);
	int			i, maxi, maxfd, listenfd, connfd, sockfd;
	int			nready, client[FD_SETSIZE];
	ssize_t			n;
	fd_set			rset, allset;
	char			buf[MAXLINE];
	socklen_t		clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

        /* SO_REUSEADDR allows a new server to be started
         * on the same port as an existing server that is
         * bound to the wildcard address, as long as each
         * instance binds a different local IP address.
         * This is common for a site hosting multiple HTTP
         * servers using the IP alias technique */
        int reuseaddr_on = 1;
        if( setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR,
                &reuseaddr_on, sizeof( reuseaddr_on)) < 0)
        {
            // log
        }

	Bind( listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen( listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for ( i = 0; i < FD_SETSIZE; ++i)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO( &allset);
	FD_SET( listenfd, &allset);

	for ( ; ; ) {
		rset = allset;                  /* structure assignment */
                
                /* select waits for something to happen: either the establishment
                 * of a new client connection or the arrival of data, a FIN,
                 * or an RST on an existing connection */
		nready = Select( maxfd + 1, &rset, NULL, NULL, NULL);

		if ( FD_ISSET( listenfd, &rset)) {	/* new client connection */
                    
                    /* simulate busy server */
                    printf( "listening socket readable -> sleep(5)\n");
                    sleep(5);

                    /* If the listening socket is readable, a new connection has been 
                     * established. We call accept and update our data structures
                     * accordingly. We use the first unused entry in the client array
                     * to record the connected socket. The number of ready descriptors
                     * is decremented, and if it is 0, we can avoid the next for loop.
                     * This lets us use the return value from select to avoid checking
                     * descriptors that are not ready.*/
			clilen = sizeof( cliaddr);
                        printf( "accept called\n"); fflush( stdout);
			connfd = accept( listenfd, ( SA *) &cliaddr, &clilen);
                        printf( "accept returned, connfd=%d\n", connfd); fflush( stdout);

			for ( i = 0; i < FD_SETSIZE; ++i)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if ( i == FD_SETSIZE)
				err_quit( "too many clients");

			FD_SET( connfd, &allset);	/* add new descriptor to set */
			if ( connfd > maxfd)
				maxfd = connfd;		/* for select */
			if ( i > maxi)
				maxi = i;		/* max index in client[] array */

			if ( --nready <= 0)
				continue;		/* no more readable descriptors */
		}

		for ( i = 0; i <= maxi; ++i) {	/* check all clients for data */
                    
                    /* A test is made for each existing client connection as to whether
                     * or not its descriptor is in the descriptor set returned by select.
                     * If so, a line is read from the client and echoed back to the client.
                     * If the client closes the connection, read returns 0 and we update
                     * our data structures accordingly. We never decrement the value
                     * of maxi, but we could check for this possibility each time a client
                     * closes its connection.*/
			if ( ( sockfd = client[i]) < 0)
				continue;
			if ( FD_ISSET( sockfd, &rset)) {
				if ( ( n = Read( sockfd, buf, MAXLINE)) == 0) {
					/* connection closed by client */
					Close( sockfd);
					FD_CLR( sockfd, &allset);
					client[i] = -1;
				} else
					Writen( sockfd, buf, n); /* echo */

				if ( --nready <= 0)
					break;		/* no more readable descriptors */
			}
		}
	}

    return 0;
}