/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on April 26, 2014, 5:02 PM
 */

extern "C" {
    #include "../../../include/unix_networking/unpv13e/lib/unp.h"
}
#include <time.h>
#include <assert.h>

int
main( int argc, char **argv) {
    int sockfd, n;
    int rc = 0;
    char buff[ MAXLINE + 1];
    char recvline[ MAXLINE + 1];
    struct sockaddr_in servaddr;
    int port = 7496;

    if ( argc != 2)
        err_quit("usage: a.out <IPaddress>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero( &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( port); /* test */
    
    if ( inet_pton( AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_quit("inet_pton error for %s", argv[1]);

    /* starting three way handshake*/
    fprintf( stderr, "starting three way handshake\n");
    if( connect( sockfd, (SA *) & servaddr, sizeof (servaddr)) < 0) {
        if( errno == ETIMEDOUT) {
            fprintf( stderr, "connection timed out, no response to SYN segment\n");
        }
        if( errno == ECONNREFUSED) {
            fprintf( stderr, "no process listening on port %d\n", port);
        }
        if( errno == EHOSTUNREACH || errno == ENETUNREACH) {
            fprintf( stderr, "can't reach the server\n", port);
        }
        err_sys( "connect error");
    }
    fprintf( stderr, "connected\n");
    
    int myInt = 60;    
    int tmp = htonl( (uint32_t)myInt);
    Write( sockfd, &tmp, sizeof(tmp));
    
    
        if ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* null terminate */
        //printf( "%s", recvline);
        if ( fputs(recvline, stdout) == EOF)
            err_sys( "fputs error");
        /*
        Since the stream is fully-buffered by default, not line-buffered,
        it needs to be flushed periodically.  We'll flush it here for
        demonstration purposes, even though we're about to close it.
         */
        int flush_status = fflush( stdout);
        if ( flush_status != 0) {
            puts( "\nError flushing stream!");
        } else {
            puts( "\nStream flushed.");
        }
    }
    if (n < 0)
        err_sys( "read error");
    
    fprintf( stderr, "exiting\n");
    exit(0);
}

