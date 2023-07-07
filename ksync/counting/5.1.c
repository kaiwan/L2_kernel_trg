/*
 * Src: Is Parallel Programming Hard, And, If So, What Can You Do About It? Paul McKenny
 */
#include <stdio.h>

unsigned long long counter = 0;

static __inline__ void inc_count(void)
{
	counter++;
	//WRITE_ONCE(counter, READ_ONCE(counter) + 1);
}

static __inline__ unsigned long long read_count(void)
{
	return counter;
	//return READ_ONCE(counter);
}

unsigned long long COUNT=1000000000L; // (10*1024*1024*1024)L;

main()
{
	unsigned long i;

	for (i = 0; i < COUNT; i++)
		inc_count();
	printf("COUNT=      %llu\ncounter val=%llu\n", COUNT, read_count());
}
