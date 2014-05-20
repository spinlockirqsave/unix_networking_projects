/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on May 7, 2014, 12:38 PM
 */


extern "C" {
    #include "../../../include/unix_networking/unpv13e/lib/unp.h"
}

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
str_echo(int sockfd)
{
	long		arg1, arg2;
	ssize_t		n;
	char		line[MAXLINE];

	for ( ; ; ) {
		if ( ( n = Readline(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

		if ( sscanf( line, "%ld%ld", &arg1, &arg2) == 2)
			snprintf( line, sizeof(line), "%ld\n", arg1 + arg2);
		else
			snprintf( line, sizeof(line), "input error\n");

		n = strlen( line);
		Writen( sockfd, line, n);
	}
}

/*
 * 
 */
int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in              cliaddr, servaddr;

	listenfd = Socket( AF_INET, SOCK_STREAM, 0);

	bzero( &servaddr, sizeof( servaddr));
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

	Bind( listenfd, (SA *) &servaddr, sizeof( servaddr));

	Listen( listenfd, LISTENQ);

	Signal( SIGCHLD, sig_chld);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ( ( connfd = accept( listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if ( errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys( "accept error");
		}

		if ( ( childpid = Fork()) == 0) {	/* child process */
			Close( listenfd);               /* close listening socket */
			str_echo( connfd);              /* process the request */
			exit( 0);
		}
		Close( connfd);			/* parent closes connected socket */
	}
}

