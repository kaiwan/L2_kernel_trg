/*
 * Src: Is Parallel Programming Hard, And, If So, What Can You Do About It? Paul McKenny
 *
 * (What does this v simple program intend to do?
 * Just this: have ONE thread - main() simply increment a GLOBAL variable a number of times
 * That's it.)
 *
 * The single-threaded version - this one - works fine!
 * Not so, the multithreaded one: see and try counting_5.1.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

unsigned long long COUNT=1000000000L; // 10^9 // (10*1024*1024*1024)L;

int main()
{
	unsigned long long i;

	for (i = 0; i < COUNT; i++)
		inc_count();
	printf("COUNT=      %llu\ncounter val=%llu\n", COUNT, read_count());
	exit(0);
}
