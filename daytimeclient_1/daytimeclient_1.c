#include "../../../include/unix_networking/unpv13e/lib/unp.h"

#include <time.h>
#include <assert.h>

int
main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("usage: a.out <IPaddress>");

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket error");

    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( 9997); /* test */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_quit("inet_pton error for %s", argv[1]);

    if (connect(sockfd, (SA *) & servaddr, sizeof (servaddr)) < 0)
        err_sys("connect error");

    int counter = 0;
    while ( ( n = read(sockfd, recvline, MAXLINE)) > 0) {
        counter = counter + 1;
        recvline[n] = 0; /* null terminate */
        //printf( "%s", recvline);
        if (fputs(recvline, stdout) == EOF)
            err_sys("fputs error");
        /*
        Since the stream is fully-buffered by default, not line-buffered,
        it needs to be flushed periodically.  We'll flush it here for
        demonstration purposes, even though we're about to close it.
         */
        int flush_status = fflush(stdout);
        if ( flush_status != 0) {
            puts("\nError flushing stream!");
        } else {
            puts("\nStream flushed.");
        }
        //printf( "counter:%d", counter);
    }
    
    printf( "counter:%d\n", counter);
    if (n < 0)
        err_sys("read error");

    exit(0);
}