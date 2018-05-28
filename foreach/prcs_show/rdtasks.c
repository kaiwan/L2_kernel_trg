/*
 * rd_tst.c
 * Test bed for demo drivers
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 *
 *
 * Released under the terms of the MIT License.
 *  https://en.wikipedia.org/wiki/MIT_License
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#define FLGS		O_RDONLY
#define DMODE		0
#define SZ		1024


void sig( int signum )
{
	fprintf (stderr,"In sig: signum=%d\n", signum);
}

int main(int argc, char **argv)
{
	int fd,n;
	struct sigaction act;
	char *buf=NULL;
	size_t num=0;
	
	if( argc!=3 ) {
		fprintf(stderr,"Usage: %s device_file num_bytes_to_read\n", argv[0]);
		exit(1);
	}

	act.sa_handler = sig;
	act.sa_flags = SA_RESTART;
	sigemptyset (&act.sa_mask);
	if ((sigaction (SIGINT, &act, 0)) == -1) {
		perror("sigaction"), exit (1);
	}	

	if( (fd=open(argv[1],FLGS,DMODE)) == -1)
		perror("open"),exit(1);
	printf("device opened: fd=%d\n",fd);

	num = atoi(argv[2]);
	if ((num < 0) || (num > INT_MAX)) {
		fprintf(stderr,"%s: number of bytes '%ld' invalid.\n", argv[0], num);
		close (fd);
		exit (1);
	}

	buf = malloc(num);
	if (!buf) {
		fprintf(stderr,"%s: out of memory!\n", argv[0]);
		close (fd);
		exit (1);
	}

	// test reading..
	n=read(fd,buf,num);
	if( n < 0 ) { perror("read failed");free(buf);close(fd);exit(1);}
	buf[n-1]='\0'; 

	//buf[n]='\0'; 
	/*
	Interesting! If the above line is compiled in, we get this error:
	*** glibc detected *** ./rd_tst: double free or corruption (!prev): 0x097e6008 ***
	======= Backtrace: =========
	/lib/libc.so.6(+0x6c501)[0x505501]
	/lib/libc.so.6(+0x6dd70)[0x506d70]
	...

	Actually, there is a bug if we keep that line: a buffer overrun by 1 byte 
	(valgrind caught it!)
	*/

	printf("%s: read %d bytes from %s\n",argv[0], n, argv[1]);
	printf("\
----------------------------------------------------------------------\n\
Task Name       | TGID  |  PID  |  RUID |  EUID\n\
----------------------------------------------------------------------\n"
	);
    close(fd);

	printf("%s\n", buf);

	free(buf);
	exit(0);       
}
