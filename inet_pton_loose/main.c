/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on April 15, 2014, 12:30 AM
 */
#include "../../../include/unix_networking/unpv13e/lib/unp.h"
//#include <cstdlib>

#define	IN6ADDRSZ	16
#define	INADDRSZ	 4
#define	INT16SZ		 2



/* int
 * inet_pton(af, src, dst)
 *	convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * return:
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *	Paul Vixie, 1996.
 */
int
inet_pton_loose(int af, const char* src, void* dst)
{
    int rc;
	switch (af) {
	case AF_INET:
	    if ( ( rc = inet_pton( AF_INET, src, (u_char*)dst)) < 0)
                return (-1);
            if( rc == 0) {
                struct in_addr addr;
                if( ( rc = inet_aton( src, &addr)) == 0)
                    return (-1);
                memcpy( dst, &addr, sizeof( struct in_addr));
                return (1);
            }
            return (1);
	case AF_INET6:
		return (inet_pton( AF_INET6, src, (u_char*)dst));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
	/* NOTREACHED */
}
        
/*
 * 
 */
int main(int argc, char** argv) {

    const char* ap = "0x1";
    struct in_addr addr;
    int rc = inet_pton_loose( AF_INET, ap, &addr);
    
    return 0;
}





