/*
 * counting_5.1.c
 * Src: Is Parallel Programming Hard, And, If So, What Can You Do About It? Paul McKenny
 *
 * As explained in the book, this completely trivialized way of counting has a major pitfall:
 * counts are lost!
 * Eg.
$ ./counting_5.1 29001

counter val=28638
 * 
 */
#define _POSIX_C_SOURCE    200112L	/* or earlier: 199506L */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_THREADS    127000 // 50000	// arbitrary
typedef unsigned int u32;
typedef unsigned long u64;

char gbuf[100];
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

void *countup(void *threadnum)
{
	sleep(2);  // IMP! ...to simulate / generate concurrency...
	//printf(".");
	inc_count();
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	pthread_t *threads;
	pthread_attr_t attr;
	int numthrds, i, ret;
	long t;
	void *stat = 0;

	if (argc != 2) {
		printf("Usage: %s num-threads-to-create (each thread counts up by 1)\n", argv[0]);
		exit(1);
	}
	numthrds = atoi(argv[1]);
	if ((numthrds <= 0) || (numthrds > MAX_THREADS)) {
		printf("%s: num-threads invalid (range: [1..MAX_THREADS(=%d)])\n", argv[0], MAX_THREADS);
		exit(1);
	}

	threads = malloc(sizeof(pthread_t) * numthrds);
	if (!threads) {
		printf("%s: out of memory!\n", argv[0]);
		exit(1);
	}

	/* Init the thread attribute structure to defaults */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (t = 0; t < numthrds; t++) {
		int rc;
		rc = pthread_create(&threads[t], &attr, countup, (void *)t);
		if (rc) {
			printf
			    ("%s: Thrd # %ld: ERROR: return code from pthread_create() is %d\nstrerror() says: %s\n",
			     argv[0], t, rc, strerror(rc));
			perror("perror(): pthread_create failed");
			exit(1);
		}
	}
	pthread_attr_destroy(&attr);
	//pause();
	/* Thread join loop */
	for (i = 0; i < numthrds; i++) {
		ret = pthread_join(threads[i], (void **)&stat);
		if (ret)
			printf("WARNING! [%d] pthread_join() failed! [%d]\n", i, ret);
		else {
			//printf(" Thread #%d successfully joined; it "
			//       "terminated with status=%ld\n", i, (long)stat);
			if (stat == PTHREAD_CANCELED)
				printf("  : was CANCELED\n");
		}
	}
	free(threads);

	printf("\ncounter val=%llu\n", read_count());
	//printf("\nCOUNT=      %llu\ncounter val=%llu\n", COUNT, read_count());
	pthread_exit(NULL);
}
