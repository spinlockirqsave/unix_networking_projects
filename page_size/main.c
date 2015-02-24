#include<stdio.h>
#include<unistd.h>

int main()
{
    int sz = getpagesize();
    printf( "page size:%d bytes\n", sz);

    return 0;
}
