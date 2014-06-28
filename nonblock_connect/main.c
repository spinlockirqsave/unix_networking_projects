/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 27, 2014, 18:12 PM
 * 
 * nonblocking connect
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "networking_functions.h"





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
            return -1;
        
        sleep(10);

	exit(0);
}