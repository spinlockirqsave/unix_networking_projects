#include "../../../include/unix_networking/unpv13e/lib/unp.h"

#include <time.h>
#include <assert.h>
struct A{ int i;};
void insert( struct A *array ) {
    printf("char:%d", array[0].i);
    printf("char:%d", array[1].i);
}
int
main(int argc, char **argv) {
    struct A i1 = {1}; struct A i2 = {2};
    struct A * m[ 2]={ &i1, &i2};
    printf("char:%d", (*m)[1].i);
    printf("char:%d", (*m[1]).i);
    insert( m);

    int listenfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buff[MAXLINE];
    time_t ticks;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9997); /* test*/

    Bind(listenfd, (SA *) & servaddr, sizeof (servaddr));

    Listen(listenfd, LISTENQ);
    puts("listening...");

    for (;;) {
        len = sizeof (cliaddr);
        connfd = Accept(listenfd, (SA *) & cliaddr, &len);
        printf( "connection from %s, port %d\n",
                Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof (buff)),
                ntohs(cliaddr.sin_port));

        ticks = time(NULL);
        snprintf(buff, sizeof (buff), "%.24s\r\n", ctime(&ticks));

        //Write(connfd, buff, strlen(buff)); // normal version, write at once
        int i; // call Write for each one byte
        for (i = 0; i < strlen(buff); ++i) {
            Write(connfd, &buff[i], 1);
        }
        printf( "writes: %d\n", i);
        Close(connfd);
    }
}
