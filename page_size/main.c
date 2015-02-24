#include<stdio.h>
#include<unistd.h>

int main()
{
    int sz = getpagesize();
    printf( "page size:%d bytes\n", sz);
    printf( "page size:%ld bytes\n", sysconf( _SC_PAGESIZE));

    return 0;
}
