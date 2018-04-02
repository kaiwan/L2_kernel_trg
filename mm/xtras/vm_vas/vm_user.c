#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef unsigned int u32;
typedef long unsigned int u64;

int g=5, u;

int main(int argc, char **argv)
{
	int local;
	char *heapptr = malloc(100);

	// OS bits
	// for cpu use 'lscpu'
	//printf("wordsize : %d\n", __WORDSIZE);
	
	if (__WORDSIZE == 32) {
		printf("32-bit: &main = 0x%08x &g = 0x%08x &u = 0x%08x &heapptr = 0x%08x &loc = 0x%08x\n", 
			(u32)main, (u32)&g, (u32)&u, (u32)heapptr, (u32)&local);
	} else if (__WORDSIZE == 64) {
		printf("64-bit: &main = 0x%016lx &g = 0x%016lx &u = %016lx &heapptr = 0x%016lx &loc = 0x%016lx\n", 
			(u64)main, (u64)&g, (u64)&u, (u64)heapptr, (u64)&local);
	}

	free(heapptr);
	return 0;
}

