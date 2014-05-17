/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on April 28, 2014, 11:31 AM
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
extern "C" {
    #include "../../../include/unix_networking/unpv13e/lib/unp.h"
}
/*
 * 
 */
int main(int argc, char** argv) {

    	int				listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in              cliaddr, servaddr;
        
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        
        bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(9877);//SERV_PORT);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	int i = listen(listenfd, LISTENQ);
        
    return 0;
}

