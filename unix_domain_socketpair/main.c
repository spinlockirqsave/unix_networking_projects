/* 
 * File:   main.c
 * Author: piter cf16 eu
 *
 * Created on June 19, 2014, 3:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

void child(int socket) {
    
    char hello[45];
    snprintf( hello, 45, "hello parent, I am child with pid=%d", getpid());
    write( socket, hello, sizeof(hello)); /* NB. this includes nul */
}

void parent(int socket) {
    
    char buf[1024];
    int n = read(socket, buf, sizeof(buf));
    printf( "parent with pid= %d received '%.*s'\n", getpid(), n, buf);
    sleep(15);
}

void socketfork() {
    int fd[2];
    const int parentsocket = 0;
    const int childsocket = 1;
    pid_t pid;

    socketpair( PF_LOCAL, SOCK_STREAM, 0, fd);


    pid = fork();
    
    if ( pid == 0) { 
        /* you are the child */
        close( fd[parentsocket]); /* Close the parent file descriptor */
        child( fd[childsocket]);
    } else { 
        /* you are the parent */
        close( fd[childsocket]); /* Close the child file descriptor */
        parent( fd[parentsocket]);
        waitpid( pid);
    }

    exit(0);
}

/*
 * 
 */
int main(int argc, char** argv) {

    socketfork();
    return (EXIT_SUCCESS);
}

/*
 * result wihout calls to close(socket)
./a.out
parent with pid= 30125 received 'hello parent, I am child with pid=30126'

me@comp:# netstat --unix -p | grep 30125
unix  3      [ ]         STREAM     CONNECTED     3043372  30125/a.out         
unix  3      [ ]         STREAM     CONNECTED     3043371  30125/a.out      call to close would result in only single descriptor CONNECTED   
me@comp:# netstat --unix -p | grep 30126
me@comp:# 
 */

