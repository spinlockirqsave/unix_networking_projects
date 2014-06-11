#include	"networking_functions.h"
#include	<syslog.h>

#define	MAXFD	64

extern int	daemon_proc;	/* defined in error.c */

int
daemon_init( const char *pname, int facility)
{
	int		i;
	pid_t	pid;

	if ( ( pid = Fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			/* parent terminates */

	/* child 1 continues... */
        /* 
         * setsid is a POSIX function that creates a new session. (Chapter 9 of APUE talks about
         * process relationships and sessions in detail.) The process becomes the session leader of the
         * new session, becomes the process group leader of a new process group, and has no
         * controlling terminal. 
         */
	if ( setsid() < 0)			/* become session leader */
		return (-1);

        /* We ignore SIGHUP and call fork again. When this function returns, the parent is really
         * the first child and it terminates, leaving the second child running. The purpose of this second
         * fork is to guarantee that the daemon cannot automatically acquire a controlling terminal
         * should it open a terminal device in the future. When a session leader without a controlling
         * terminal opens a terminal device (that is not currently some other session's controlling
         * terminal), the terminal becomes the controlling terminal of the session leader. But by calling
         * fork a second time, we guarantee that the second child is no longer a session leader, so it
         * cannot acquire a controlling terminal. We must ignore SIGHUP because when the session
         * leader terminates (the first child), all processes in the session (our second child) receive the
         * SIGHUP signal.
         */
	Signal( SIGHUP, SIG_IGN);
	if ( ( pid = Fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			/* child 1 terminates */

	/* child 2 continues... */
        /*
         * We set the global daemon_proc to nonzero. This external is defined by our err_XXX
         * functions (Section D.3), and when its value is nonzero, this tells them to call syslog instead
         * of doing an fprintf to standard error. This saves us from having to go through all our code
         * and call one of our error functions if the server is not being run as a daemon (i.e., when we
         * are testing the server), but call syslog if it is being run as a daemon.
         */
	daemon_proc = 1;			/* for err_XXX() functions */

	chdir("/");				/* change working directory */

	/* close off file descriptors */
	for (i = 0; i < MAXFD; i++)
		close(i);

	/* redirect stdin, stdout, and stderr to /dev/null */
	open( "/dev/null", O_RDONLY);
	open( "/dev/null", O_RDWR);
	open( "/dev/null", O_RDWR);

        /* 
         * will log into /var/log/syslog as default, 
         * i.e. Jun 11 23:40:27 ubuntucomputer ./deamon_init[23214]: connection from 127.0.0.1:44937
         * please redirect output from pname in /etc/rsyslog.conf
         * i.e kern.* local7.debug
         * /dev/console /var/log/cisco.log
         */
	openlog( pname, LOG_PID, facility);

	return (0);				/* success */
}
