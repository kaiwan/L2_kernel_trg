/*
 * src: http://vulnfactory.org/blog/2013/02/06/a-linux-memory-trick/
 *
It 'should' segfault. Once it does, look up dmesg:

"... deliberately causes an access violation on a mapped kernel address,
resulting in an error code of 5 (a read violation from user mode on a
present page). The second invocation causes an access violation on an
unmapped kernel address, resulting in an error code of 4 (a read
violation from user mode on a non-present page). ..."
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int *ptr, foo;
	if (argc ==1) {
	  fprintf(stderr,"Usage: %s kernel|user-va\n", argv[0]);
	  exit(1);
	}
    ptr = (int *)strtoul(argv[1], NULL, 16);
    foo = *ptr;
}
