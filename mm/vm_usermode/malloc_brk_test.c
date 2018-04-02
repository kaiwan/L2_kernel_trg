/*
 * malloc_brk_test.c

Rationale:
----------
The first time a process does an malloc(8) :

- the heap memory is not physically allocated yet, only virtually 
  (remember, the kernel is a lazy guy!).
- Thus, when attempting to map the virtual to physical address, the 
  MMU faults; the kernel “realizes” this is a good fault and allocates 
  a page farme from the slab cache (possibly via the buddy allocator). 
  This is mapped into the faulting process’s heap..
- Another request is made, say:
  malloc(32);
- Now, since the glibc memory manager (what we call malloc) now knows 
  that a full page is available in the heap and only 8 bytes are actually 
  used up, this request will not cause a brk() syscall to the kernel. This 
  can be verified by checking the current break (using sbrk(0)).

- Also, seeking to within a page of the 1st request will (probably) not cause 
  a segfault!

  See below a program that helps us study how the system allocates memory 
  dynamically to user-space applications via the usual malloc / calloc API.

(c) Kaiwan NB.
GPL v2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>

//#define USE_CALLOC
#undef USE_CALLOC

#define TRIES 5
void *heap_ptr[TRIES];
void *init_brk;

typedef unsigned int u32;

static void alloctest(int index, size_t num)
{
	void *p;

#ifndef USE_CALLOC
	p = malloc(num);
#else
	p = calloc(num, sizeof(char));
#endif
	if (!p) {
		printf("out of memory!\n");
		exit(1);
	}

	heap_ptr[index] = p;	// save ptr in order to free later..
#ifndef USE_CALLOC
	printf("\n%d: malloc", index);
#else
	printf("\n%d: calloc", index);
#endif
	printf("(%6u) successful. Heap pointer: %8p\n", num, p);
	printf("Current break: %8p [delta: %d]\n", sbrk(0),
	       (sbrk(0) - init_brk));

	malloc_stats();
}

int main(int argc, char **argv)
{
	int i;
	volatile char *q;

	init_brk = sbrk(0);
	printf("Current break: %8p\n", init_brk);
	alloctest(0, 8);

//exit(0);

	q = heap_ptr[0];
	*(q + 3000) = 'a';	/* "should" segfault but does (probably) not bcoz a *page* is alloced 
				   by the previous alloc, not just 8 bytes! See value of prg break
				   compared to this pointer.
				 */
	printf
	    ("\n### q=0x%08x. (q+3000) is the mem loc 0x%08x. Mem here is: 0x%08x\n",
	     (unsigned int)q, (unsigned int)(q + 3000),
	     (unsigned int)*(q + 3000));
	/*
	   Indeed valgrind points out the errors:

	   $ valgrind --leak-check=full ./malloc_brk_test
	   ...
	   0: malloc(     8) successful. Heap pointer: 0x41f7028
	   Current break: 0x804b000 [delta: 0]
	   ==31220== Invalid write of size 1
	   ==31220==    at 0x804863E: main (malloc_brk_test.c:80)
	   ==31220==  Address 0x41f7be0 is not stack'd, malloc'd or (recently) free'd
	   ==31220== 
	   ==31220== Invalid read of size 1
	   ==31220==    at 0x804864A: main (malloc_brk_test.c:85)
	   ==31220==  Address 0x41f7be0 is not stack'd, malloc'd or (recently) free'd
	   ==31220== 

	   ### q=0x041f7028. (q+3000) is the mem loc 0x041f7be0. Mem here is: 0x00000061
	   ...
	 */

#if 0
	*(q + 3000 + (sbrk(0) - init_brk)) = 'b';	/* *make* it segfault here by poking into a region 
							   just beyond what the kernel allocated */
#endif

//pause();
//exit(0);

	alloctest(1, (getpagesize() - 8 - 5));
	alloctest(2, 3);
//exit(0);
	alloctest(3, (sbrk(0) - init_brk + 1000));
	/* This (above) allocation request is a large one: ~132Kb. The 'mmap threshold' 
	   is (default) 128Kb; thus, this causes an mmap() to the process virtual address 
	   space, mapping in the virtually allocated region (which will later be mapped to 
	   physical page frames via the MMU page-faulting on application access to these memory regions!
	 */
	alloctest(4, 200000);

	for (i = 0; i < TRIES; i++)
		free(heap_ptr[i]);
	exit(0);
}

/* vi: ts=4 */
