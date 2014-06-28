/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 27, 2014, 18:12 PM
 * 
 * Nonblocking connect
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"



void
str_cli( FILE *fp, int sockfd)
{
	pid_t	pid;
	char	sendline[MAXLINE], recvline[MAXLINE];

	if ( ( pid = Fork()) == 0) {		
            
            /* child: server -> stdout */
            int n;
            while ( ( n = Read( sockfd, recvline, MAXLINE)) > 0)
            {
                if( n < MAXLINE)
                    recvline[n] = 0;
                else recvline[ MAXLINE - 1] = 0;
                
                Fputs( recvline, stdout);
            }

            /* in case parent still running */
            kill( getppid(), SIGTERM);
            exit(0);
	}

		/* parent: stdin -> server */
	while ( Fgets( sendline, MAXLINE, fp) != NULL)
		Writen( sockfd, sendline, strlen(sendline));

        /* received EOF on stdin, send FIN */
	Shutdown( sockfd, SHUT_WR);
	pause();
	return;
}

/*
 * 
 */
int
main( int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_in	servaddr;

	if ( argc != 2)
		err_quit( "usage: tcpcli <IPaddress>");

	sockfd = Socket( AF_INET, SOCK_STREAM, 0);

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	Inet_pton( AF_INET, argv[1], &servaddr.sin_addr);

	if( connect_nonblocking( sockfd, (SA *) &servaddr, sizeof(servaddr), 0) < 0)
            err_sys("connect error");

        str_cli( stdin, sockfd);		/* do it all */

	exit(0);
}