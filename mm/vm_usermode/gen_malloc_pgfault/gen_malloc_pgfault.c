/*
 * gen_malloc_pgfault.c
 *
 * When a process allocates a page with malloc(), it's only allocated
 * *virtually*. When it touches the memory (rd/wr), the MMU triggers a
 * page fault; the OS's fault handler is sophisticated: it figures out 
 * that this is a legal access; thus it allocates physical memory (via 
 * the buddy system allocator) and fixes up the paging tables (for current,
 * such that the vp -> pf !
 *
 * This simple app emulates this malloc & touching of memory.
 * Try tracing the kernel - the page fault handler - via tracing tools like
 * ftrace.
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): [L]GPL
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/*---------------- Macros -------------------------------------------*/


/*---------------- Typedef's, constants, etc ------------------------*/


/*---------------- Functions ----------------------------------------*/


int main (int argc, char **argv)
{
	char *p=0;

	p = malloc(getpagesize()); // typically 4k
	if (!p) {
	  fprintf(stderr, "%s: out of memory!", argv[0]);
	  exit(1);
	}
	*(p+200)='a';

	free(p);
	exit (0);
}
