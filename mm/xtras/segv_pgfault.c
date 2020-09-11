/*
 * segv_pgfault.c
 *
 * Make a usermode process segfault by accessing invalid user/kernel-space addresses..
 * This in turn will have the MMU trigger an exception condition (Data Abort on 
 * ARM), which will lead to the OS's page fault handler being invoked. *It* will 
 * determine the actual fault (minor or major, good or bad) and, in this case, being
 * a usermode 'bad' fault, will send SIGSEGV to 'current'!
 *
 * Author(s) :  Kaiwan NB
 * License(s): MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

/*---------------- Typedef's, constants, etc ------------------------*/
typedef unsigned int u32;
typedef long unsigned int u64;

/*---------------- Macros -------------------------------------------*/
#if __x86_64__   /* 64-bit; __x86_64__ works for gcc */
 #define ADDR_TYPE u64
 #define ADDR_FMT "%016lx"
 static u64 rubbish_uaddr = 0x100000L;
 static u64 rubbish_kaddr = 0xffff0a8700100000L;
#else
 #define ADDR_TYPE u32
 #define ADDR_FMT "%08lx"
 static u32 rubbish_uaddr = 0x100000L;
 static u32 rubbish_kaddr = 0xd0100000L;
#endif


/*---------------- Functions ----------------------------------------*/
static void myfault(int signum, siginfo_t * siginfo, void *rest)
{
	static int c = 0;

	printf("*** %s: [%d] received signal %d. errno=%d\n"
	       " Cause/Origin: (si_code=%d): ",
	       __func__, ++c, signum, siginfo->si_errno, siginfo->si_code);

	switch (siginfo->si_code) {
	case SI_USER:
		printf("user\n");
		break;
	case SI_KERNEL:
		printf("kernel\n");
		break;
	case SI_QUEUE:
		printf("queue\n");
		break;
	case SI_TIMER:
		printf("timer\n");
		break;
	case SI_MESGQ:
		printf("mesgq\n");
		break;
	case SI_ASYNCIO:
		printf("async io\n");
		break;
	case SI_SIGIO:
		printf("sigio\n");
		break;
	case SI_TKILL:
		printf("t[g]kill\n");
		break;
		// other poss values si_code can have for SIGSEGV
	case SEGV_MAPERR:
		printf("SEGV_MAPERR: address not mapped to object\n");
		break;
	case SEGV_ACCERR:
		printf("SEGV_ACCERR: invalid permissions for mapped object\n");
		break;
	default:
		printf("-none-\n");
	}
	printf(" Faulting addr=0x" ADDR_FMT "\n", (ADDR_TYPE) siginfo->si_addr);

#if 1
	exit (1);
#else
	abort();
#endif
}

static void usage(char *nm)
{
	fprintf(stderr, "Usage: %s u|k r|w\n"
		"u => user mode\n"
		"k => kernel mode\n"
		" r => read attempt\n" " w => write attempt\n", nm);
}


int main(int argc, char **argv)
{
	struct sigaction act;

	if (argc != 3) {
		usage(argv[0]);
		exit(1);
	}
	//act.sa_handler = myfault;
	act.sa_sigaction = myfault;
	act.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("sigaction");
		exit(1);
	}

	if ((tolower(argv[1][0]) == 'u') && tolower(argv[2][0] == 'r')) {
		ADDR_TYPE *uptr = (ADDR_TYPE *) rubbish_uaddr;	// arbitrary userspace virtual addr
		printf
		    ("Attempting to read contents of arbitrary usermode va uptr = 0x" ADDR_FMT ":\n",
		     (ADDR_TYPE) uptr);
		printf("*uptr = 0x" ADDR_FMT "\n", *uptr);	// just reading
	} else if ((tolower(argv[1][0]) == 'u') && tolower(argv[2][0] == 'w')) {
		ADDR_TYPE *uptr = (ADDR_TYPE *) & main;
		printf
		    ("Attempting to write into arbitrary usermode va uptr (&main actually) = 0x" ADDR_FMT ":\n",
		     (ADDR_TYPE) uptr);
		*uptr = 40;	// writing
	} else if ((tolower(argv[1][0]) == 'k') && tolower(argv[2][0] == 'r')) {
		ADDR_TYPE *kptr = (ADDR_TYPE *) rubbish_kaddr; 	// arbitrary kernel virtual addr
		printf
		    ("Attempting to read contents of arbitrary kernel va kptr = 0x" ADDR_FMT ":\n",
		     (ADDR_TYPE) kptr);
		printf("*kptr = 0x" ADDR_FMT "\n", *kptr);	// just reading
	} else if ((tolower(argv[1][0]) == 'k') && tolower(argv[2][0] == 'w')) {
		ADDR_TYPE *kptr = (ADDR_TYPE *) rubbish_kaddr; 	// arbitrary kernel virtual addr
		printf
		    ("Attempting to write into arbitrary kernel va kptr = 0x" ADDR_FMT ":\n",
		     (ADDR_TYPE) kptr);
		*kptr = 0x62;	// writing
	} else
		usage(argv[0]);
	exit(0);
}

/* vi: ts=4 */
