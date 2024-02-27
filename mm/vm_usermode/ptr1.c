#include <stdio.h>
#include <unistd.h>

int *gp = (int *)0x42000;
int g = 0x42;

int main()
{
	int loc = 0x1;

	printf("&gp = %p, &g=%p &loc=%p\n", gp, &g, &loc);
#if 0
	printf("val = %x\n", *gp);  // segfaults; ptrs have no memory
#endif
	pause();

}
