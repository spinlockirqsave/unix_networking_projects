#include "../../../include/unix_networking/unpv13e/lib/unp.h"

#include <time.h>

int
main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    time_t ticks;
    int connections[4];
    time_t times[4];

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons( 7777); /* daytime server */

    Bind(listenfd, (SA *) & servaddr, sizeof ( servaddr));
    
    Listen(listenfd, LISTENQ);

    int i = 0;
    for ( ;; ++i) {
        connfd = Accept(listenfd, (SA *) NULL, NULL);     
        ticks = time(NULL);
        connections[ i] = connfd;
        times[ i] = ticks;
        if ( i == 1) break;
    }
    
    snprintf( buff, sizeof (buff), "You are 0. At time: %.24s\r\n", ctime( &times[0]));
    Write( connections[0], buff, strlen(buff));
    Close( connections[0]);
    
    snprintf( buff, sizeof (buff), "You are 1. At time: %.24s\r\n", ctime( &times[1]));
    Write( connections[1], buff, strlen(buff));
    Close( connections[1]);
    
    return 0;
}